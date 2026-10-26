#include <assert.h>
#include <defs.h>
#include <list.h>
#include <mmu.h>
#include <pmm.h>
#include <slub.h>
#include <stdio.h>
#include <string.h>

/* lab2 SLUB challenge 2413995 */

#define SLUB_MIN_SIZE          8
#define SLUB_MAX_SIZE          2048
#define SLUB_CACHE_COUNT       9
#define SLUB_BITMAP_WORDS      ((PGSIZE / SLUB_MIN_SIZE + 63) / 64)

#define SLUB_SMALL_MAGIC       0x534c5542534d414cULL
#define SLUB_LARGE_MAGIC       0x534c55424c415247ULL

struct slub_cache;

struct slub_page {
    uint64_t magic;
    struct Page *page;
    struct slub_cache *cache;
    list_entry_t link;
    void *free_list;
    size_t object_size;
    size_t object_count;
    size_t free_count;
    uint64_t allocated[SLUB_BITMAP_WORDS];
};

struct slub_cache {
    size_t object_size;
    list_entry_t partial;
    list_entry_t full;
};

struct slub_large {
    uint64_t magic;
    struct Page *page;
    size_t nr_pages;
    size_t requested_size;
};

static const size_t slub_class_sizes[SLUB_CACHE_COUNT] = {
    8, 16, 32, 64, 128, 256, 512, 1024, 2048,
};

static struct slub_cache slub_caches[SLUB_CACHE_COUNT];
static bool slub_initialized;

static size_t slub_align_up(size_t value, size_t alignment) {
    assert(alignment != 0);
    if (value > (size_t)-1 - (alignment - 1)) {
        return 0;
    }
    return ROUNDUP(value, alignment);
}

static size_t slub_small_header_size(void) {
    return slub_align_up(sizeof(struct slub_page), sizeof(void *));
}

static size_t slub_large_header_size(void) {
    return slub_align_up(sizeof(struct slub_large), sizeof(void *));
}

static void *slub_page_kva(struct Page *page) {
    uintptr_t pa = page2pa(page);
    assert(PPN(pa) < npage);
    return (void *)(pa + va_pa_offset);
}

static struct slub_cache *slub_find_cache(size_t size) {
    for (size_t i = 0; i < SLUB_CACHE_COUNT; i++) {
        if (size <= slub_caches[i].object_size) {
            return &slub_caches[i];
        }
    }
    return NULL;
}

static size_t slub_object_index(struct slub_page *slab, void *object) {
    uintptr_t start = (uintptr_t)slab + slub_small_header_size();
    uintptr_t address = (uintptr_t)object;

    assert(address >= start);
    size_t offset = address - start;
    assert(offset % slab->object_size == 0);

    size_t index = offset / slab->object_size;
    assert(index < slab->object_count);
    return index;
}

static bool slub_object_allocated(struct slub_page *slab, size_t index) {
    return (slab->allocated[index / 64] >> (index % 64)) & 1ULL;
}

static void slub_mark_allocated(struct slub_page *slab, size_t index) {
    assert(!slub_object_allocated(slab, index));
    slab->allocated[index / 64] |= 1ULL << (index % 64);
}

static void slub_mark_free(struct slub_page *slab, size_t index) {
    assert(slub_object_allocated(slab, index));
    slab->allocated[index / 64] &= ~(1ULL << (index % 64));
}

static struct slub_page *slub_create_page(struct slub_cache *cache) {
    struct Page *page = alloc_page();
    if (page == NULL) {
        return NULL;
    }

    struct slub_page *slab = (struct slub_page *)slub_page_kva(page);
    size_t header_size = slub_small_header_size();
    assert(header_size != 0 && header_size < PGSIZE);

    memset(slab, 0, header_size);
    slab->magic = SLUB_SMALL_MAGIC;
    slab->page = page;
    slab->cache = cache;
    slab->object_size = cache->object_size;
    slab->object_count = (PGSIZE - header_size) / cache->object_size;
    slab->free_count = slab->object_count;
    list_init(&slab->link);
    assert(slab->object_count != 0);
    assert(slab->object_count <= SLUB_BITMAP_WORDS * 64);

    char *object_start = (char *)slab + header_size;
    for (size_t i = 0; i < slab->object_count; i++) {
        void *object = object_start + i * cache->object_size;
        *(void **)object = slab->free_list;
        slab->free_list = object;
    }

    list_add_before(&cache->partial, &slab->link);
    return slab;
}

static void *slub_alloc_small(struct slub_cache *cache) {
    struct slub_page *slab;

    if (list_empty(&cache->partial)) {
        slab = slub_create_page(cache);
        if (slab == NULL) {
            return NULL;
        }
    } else {
        slab = to_struct(list_next(&cache->partial),
                         struct slub_page, link);
    }

    assert(slab->magic == SLUB_SMALL_MAGIC);
    assert(slab->free_count != 0 && slab->free_list != NULL);

    void *object = slab->free_list;
    slab->free_list = *(void **)object;

    size_t index = slub_object_index(slab, object);
    slub_mark_allocated(slab, index);
    slab->free_count--;

    if (slab->free_count == 0) {
        list_del_init(&slab->link);
        list_add_before(&cache->full, &slab->link);
    }
    return object;
}

static void *slub_alloc_large(size_t size) {
    size_t header_size = slub_large_header_size();
    if (header_size == 0 || size > (size_t)-1 - header_size) {
        return NULL;
    }

    size_t total = header_size + size;
    if (total > (size_t)-1 - (PGSIZE - 1)) {
        return NULL;
    }
    size_t nr_pages = ROUNDUP(total, PGSIZE) / PGSIZE;
    if (nr_pages == 0) {
        return NULL;
    }

    struct Page *page = alloc_pages(nr_pages);
    if (page == NULL) {
        return NULL;
    }

    struct slub_large *large = (struct slub_large *)slub_page_kva(page);
    memset(large, 0, header_size);
    large->magic = SLUB_LARGE_MAGIC;
    large->page = page;
    large->nr_pages = nr_pages;
    large->requested_size = size;

    return (char *)large + header_size;
}

void slub_init(void) {
    if (slub_initialized) {
        return;
    }

    size_t small_header = slub_small_header_size();
    size_t large_header = slub_large_header_size();
    assert(small_header != 0 && small_header < PGSIZE);
    assert(large_header != 0 && large_header < PGSIZE);

    for (size_t i = 0; i < SLUB_CACHE_COUNT; i++) {
        slub_caches[i].object_size = slub_class_sizes[i];
        assert(slub_caches[i].object_size >= sizeof(void *));
        assert(slub_caches[i].object_size % sizeof(void *) == 0);
        list_init(&slub_caches[i].partial);
        list_init(&slub_caches[i].full);
    }
    slub_initialized = 1;
    cprintf("SLUB: initialized 9 size classes (8..2048 bytes)\n");
}

void *slub_alloc(size_t size) {
    if (!slub_initialized || size == 0) {
        return NULL;
    }

    struct slub_cache *cache = slub_find_cache(size);
    if (cache != NULL) {
        return slub_alloc_small(cache);
    }
    return slub_alloc_large(size);
}

void slub_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    assert(slub_initialized);

    uintptr_t page_address = ROUNDDOWN((uintptr_t)ptr, PGSIZE);
    uint64_t magic = *(uint64_t *)page_address;

    if (magic == SLUB_SMALL_MAGIC) {
        struct slub_page *slab = (struct slub_page *)page_address;
        struct slub_cache *cache = slab->cache;
        assert(cache >= slub_caches &&
               cache < slub_caches + SLUB_CACHE_COUNT);

        size_t index = slub_object_index(slab, ptr);
        bool was_full = (slab->free_count == 0);
        slub_mark_free(slab, index);

        *(void **)ptr = slab->free_list;
        slab->free_list = ptr;
        slab->free_count++;
        assert(slab->free_count <= slab->object_count);

        if (was_full) {
            list_del_init(&slab->link);
            list_add_before(&cache->partial, &slab->link);
        }

        if (slab->free_count == slab->object_count) {
            struct Page *page = slab->page;
            list_del_init(&slab->link);
            slab->magic = 0;
            free_page(page);
        }
        return;
    }

    if (magic == SLUB_LARGE_MAGIC) {
        struct slub_large *large = (struct slub_large *)page_address;
        struct Page *page = large->page;
        size_t nr_pages = large->nr_pages;
        assert(page != NULL && nr_pages != 0);
        large->magic = 0;
        free_pages(page, nr_pages);
        return;
    }

    assert(0);
}
#define SLUB_TEST_MAX_OBJECTS 80

static size_t slub_validate_list(struct slub_cache *cache,
                                 list_entry_t *head, bool full) {
    size_t page_count = 0;
    list_entry_t *le = head;

    while ((le = list_next(le)) != head) {
        assert(++page_count <= npage);

        struct slub_page *slab =
            to_struct(le, struct slub_page, link);
        assert(slab->magic == SLUB_SMALL_MAGIC);
        assert(slab->cache == cache);
        assert(slab->page != NULL);
        assert(slub_page_kva(slab->page) == slab);
        assert(slab->object_size == cache->object_size);
        assert(slab->object_count ==
               (PGSIZE - slub_small_header_size()) / slab->object_size);
        assert(slab->free_count <= slab->object_count);

        size_t allocated_count = 0;
        for (size_t i = 0; i < SLUB_BITMAP_WORDS * 64; i++) {
            bool allocated = slub_object_allocated(slab, i);
            if (i < slab->object_count) {
                allocated_count += allocated;
            } else {
                assert(!allocated);
            }
        }
        assert(allocated_count + slab->free_count ==
               slab->object_count);

        size_t free_nodes = 0;
        void *object = slab->free_list;
        while (object != NULL) {
            assert(free_nodes < slab->object_count);
            size_t index = slub_object_index(slab, object);
            assert(!slub_object_allocated(slab, index));
            object = *(void **)object;
            free_nodes++;
        }
        assert(free_nodes == slab->free_count);

        if (full) {
            assert(slab->free_count == 0);
            assert(slab->free_list == NULL);
        } else {
            assert(slab->free_count > 0);
            assert(slab->free_count < slab->object_count);
            assert(slab->free_list != NULL);
        }
    }
    return page_count;
}

static size_t slub_validate(void) {
    size_t page_count = 0;

    for (size_t i = 0; i < SLUB_CACHE_COUNT; i++) {
        struct slub_cache *cache = &slub_caches[i];
        assert(cache->object_size == slub_class_sizes[i]);
        page_count += slub_validate_list(cache, &cache->partial, 0);
        page_count += slub_validate_list(cache, &cache->full, 1);
    }
    return page_count;
}

static void slub_assert_empty(void) {
    assert(slub_validate() == 0);
    for (size_t i = 0; i < SLUB_CACHE_COUNT; i++) {
        assert(list_empty(&slub_caches[i].partial));
        assert(list_empty(&slub_caches[i].full));
    }
}

static struct slub_page *slub_test_page(void *ptr) {
    uintptr_t page_address = ROUNDDOWN((uintptr_t)ptr, PGSIZE);
    struct slub_page *slab = (struct slub_page *)page_address;
    assert(slab->magic == SLUB_SMALL_MAGIC);
    return slab;
}

static size_t slub_test_capacity(void *ptr) {
    uintptr_t page_address = ROUNDDOWN((uintptr_t)ptr, PGSIZE);
    uint64_t magic = *(uint64_t *)page_address;

    if (magic == SLUB_SMALL_MAGIC) {
        return ((struct slub_page *)page_address)->object_size;
    }
    assert(magic == SLUB_LARGE_MAGIC);
    return ((struct slub_large *)page_address)->requested_size;
}

static bool slub_test_overlap(void *left, size_t left_size,
                              void *right, size_t right_size) {
    uintptr_t left_address = (uintptr_t)left;
    uintptr_t right_address = (uintptr_t)right;
    return left_address < right_address + right_size &&
           right_address < left_address + left_size;
}

static void slub_test_assert_disjoint(void **objects, size_t count) {
    for (size_t i = 0; i < count; i++) {
        assert(objects[i] != NULL);
        size_t left_size = slub_test_capacity(objects[i]);
        for (size_t j = i + 1; j < count; j++) {
            assert(!slub_test_overlap(objects[i], left_size, objects[j],
                                      slub_test_capacity(objects[j])));
        }
    }
}

static void slub_test_fill(void *ptr, size_t size, uint8_t pattern) {
    uint8_t *bytes = (uint8_t *)ptr;
    for (size_t i = 0; i < size; i++) {
        bytes[i] = pattern;
    }
}

static void slub_test_check(void *ptr, size_t size, uint8_t pattern) {
    uint8_t *bytes = (uint8_t *)ptr;
    for (size_t i = 0; i < size; i++) {
        assert(bytes[i] == pattern);
    }
}

static void slub_test_size_classes(void) {
    const size_t sizes[] = {
        1, 8, 16, 24, 32, 64, 100, 128, 256, 512, 1024, 2048,
    };
    void *objects[sizeof(sizes) / sizeof(sizes[0])];
    size_t before = nr_free_pages();

    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        objects[i] = slub_alloc(sizes[i]);
        assert(objects[i] != NULL);
        assert(((uintptr_t)objects[i] % sizeof(void *)) == 0);

        struct slub_cache *cache = slub_find_cache(sizes[i]);
        assert(cache != NULL);
        struct slub_page *slab = slub_test_page(objects[i]);
        assert(slab->cache == cache);
        assert(slab->object_size >= sizes[i]);
        assert((uintptr_t)objects[i] >=
               (uintptr_t)slab + slub_small_header_size());
        assert((uintptr_t)objects[i] + slab->object_size <=
               (uintptr_t)slab + PGSIZE);
    }

    slub_test_assert_disjoint(objects,
                              sizeof(objects) / sizeof(objects[0]));
    slub_validate();

    for (size_t i = sizeof(objects) / sizeof(objects[0]); i > 0; i--) {
        slub_free(objects[i - 1]);
    }
    assert(nr_free_pages() == before);
    slub_assert_empty();
    cprintf("SLUB test 1 - size classes: PASS\n");
}

static void slub_test_data_integrity(void) {
    const size_t sizes[] = {8, 16, 24, 100, 300, 1024, 2048};
    void *objects[sizeof(sizes) / sizeof(sizes[0])];
    size_t before = nr_free_pages();

    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        objects[i] = slub_alloc(sizes[i]);
        assert(objects[i] != NULL);
        slub_test_fill(objects[i], sizes[i], (uint8_t)(0x21 + i * 17));
    }
    slub_test_assert_disjoint(objects,
                              sizeof(objects) / sizeof(objects[0]));

    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        slub_test_check(objects[i], sizes[i],
                        (uint8_t)(0x21 + i * 17));
    }

    slub_test_fill(objects[3], sizes[3], 0xe3);
    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        uint8_t pattern = (i == 3) ? 0xe3 : (uint8_t)(0x21 + i * 17);
        slub_test_check(objects[i], sizes[i], pattern);
    }

    for (size_t i = 0; i < sizeof(objects) / sizeof(objects[0]); i++) {
        slub_free(objects[i]);
    }
    assert(nr_free_pages() == before);
    slub_assert_empty();
    cprintf("SLUB test 2 - data integrity: PASS\n");
}

static void slub_test_object_reuse(void) {
    void *objects[8];
    void *reused[3];
    size_t before = nr_free_pages();

    for (size_t i = 0; i < 8; i++) {
        objects[i] = slub_alloc(64);
        assert(objects[i] != NULL);
        slub_test_fill(objects[i], 64, (uint8_t)(0x40 + i));
    }

    slub_free(objects[1]);
    slub_free(objects[3]);
    slub_free(objects[6]);

    reused[0] = slub_alloc(64);
    reused[1] = slub_alloc(64);
    reused[2] = slub_alloc(64);
    assert(reused[0] == objects[6]);
    assert(reused[1] == objects[3]);
    assert(reused[2] == objects[1]);

    const size_t active[] = {0, 2, 4, 5, 7};
    for (size_t i = 0; i < sizeof(active) / sizeof(active[0]); i++) {
        size_t index = active[i];
        slub_test_check(objects[index], 64, (uint8_t)(0x40 + index));
    }

    for (size_t i = 0; i < sizeof(active) / sizeof(active[0]); i++) {
        slub_free(objects[active[i]]);
    }
    for (size_t i = 0; i < 3; i++) {
        slub_free(reused[i]);
    }

    assert(nr_free_pages() == before);
    slub_assert_empty();
    cprintf("SLUB test 3 - object reuse: PASS\n");
}

static void slub_test_slab_expansion(void) {
    void *objects[SLUB_TEST_MAX_OBJECTS];
    size_t before = nr_free_pages();

    objects[0] = slub_alloc(64);
    assert(objects[0] != NULL);
    struct slub_page *first = slub_test_page(objects[0]);
    size_t count = first->object_count + 4;
    assert(count <= SLUB_TEST_MAX_OBJECTS);

    for (size_t i = 1; i < count; i++) {
        objects[i] = slub_alloc(64);
        assert(objects[i] != NULL);
    }

    bool saw_second_page = 0;
    for (size_t i = 0; i < count; i++) {
        if (slub_test_page(objects[i]) != first) {
            saw_second_page = 1;
        }
    }
    assert(saw_second_page);
    assert(nr_free_pages() == before - 2);
    assert(slub_validate() == 2);
    slub_test_assert_disjoint(objects, count);

    for (size_t i = count; i > 0; i--) {
        slub_free(objects[i - 1]);
    }
    assert(nr_free_pages() == before);
    slub_assert_empty();
    cprintf("SLUB test 4 - slab expansion: PASS\n");
}

static void slub_test_page_reclaim(void) {
    void *objects[SLUB_TEST_MAX_OBJECTS];
    size_t before = nr_free_pages();

    objects[0] = slub_alloc(256);
    assert(objects[0] != NULL);
    struct slub_page *first = slub_test_page(objects[0]);
    size_t count = first->object_count;
    assert(count + 1 <= SLUB_TEST_MAX_OBJECTS);

    slub_test_fill(objects[0], 256, 0x51);
    for (size_t i = 1; i <= count; i++) {
        objects[i] = slub_alloc(256);
        assert(objects[i] != NULL);
        if (i < count) {
            slub_test_fill(objects[i], 256, (uint8_t)(0x51 + i));
        }
    }

    assert(slub_test_page(objects[count]) != first);
    assert(nr_free_pages() == before - 2);

    slub_free(objects[count]);
    assert(nr_free_pages() == before - 1);
    assert(slub_validate() == 1);

    for (size_t i = 0; i < count; i++) {
        slub_test_check(objects[i], 256, (uint8_t)(0x51 + i));
    }

    for (size_t i = 0; i < count; i++) {
        slub_free(objects[i]);
    }
    assert(nr_free_pages() == before);
    slub_assert_empty();
    cprintf("SLUB test 5 - page reclaim: PASS\n");
}

static void slub_test_irregular_sizes(void) {
    const size_t sizes[] = {13, 100, 300, 3000, 4096, 8192};
    const size_t small_classes[] = {16, 128, 512};
    void *objects[sizeof(sizes) / sizeof(sizes[0])];
    size_t before = nr_free_pages();

    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        objects[i] = slub_alloc(sizes[i]);
        assert(objects[i] != NULL);
        assert(((uintptr_t)objects[i] % sizeof(void *)) == 0);
        slub_test_fill(objects[i], sizes[i], (uint8_t)(0x71 + i));

        uintptr_t base = ROUNDDOWN((uintptr_t)objects[i], PGSIZE);
        if (i < 3) {
            struct slub_page *slab = (struct slub_page *)base;
            assert(slab->magic == SLUB_SMALL_MAGIC);
            assert(slab->object_size == small_classes[i]);
        } else {
            struct slub_large *large = (struct slub_large *)base;
            assert(large->magic == SLUB_LARGE_MAGIC);
            assert(large->requested_size == sizes[i]);
            assert(large->nr_pages ==
                   ROUNDUP(slub_large_header_size() + sizes[i], PGSIZE) /
                       PGSIZE);
        }
    }

    slub_test_assert_disjoint(objects,
                              sizeof(objects) / sizeof(objects[0]));
    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        slub_test_check(objects[i], sizes[i], (uint8_t)(0x71 + i));
    }

    for (size_t i = sizeof(objects) / sizeof(objects[0]); i > 0; i--) {
        slub_free(objects[i - 1]);
    }
    assert(nr_free_pages() == before);
    slub_assert_empty();
    cprintf("SLUB test 6 - irregular sizes: PASS\n");
}

static void slub_test_state_consistency(void) {
    const size_t sizes[] = {1, 13, 64, 300, 2048, 3000};
    void *objects[sizeof(sizes) / sizeof(sizes[0])];
    size_t before = nr_free_pages();

    assert(slub_alloc(0) == NULL);
    slub_free(NULL);
    assert(nr_free_pages() == before);
    slub_assert_empty();

    assert(npage < (size_t)-1 / PGSIZE);
    size_t too_large = (npage + 1) * PGSIZE;
    assert(slub_alloc(too_large) == NULL);
    assert(slub_alloc((size_t)-1) == NULL);
    assert(nr_free_pages() == before);
    slub_assert_empty();

    for (size_t round = 0; round < 4; round++) {
        for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
            objects[i] = slub_alloc(sizes[i]);
            assert(objects[i] != NULL);
            slub_test_fill(objects[i], sizes[i],
                           (uint8_t)(0x10 + round * 23 + i));
        }
        slub_test_assert_disjoint(objects,
                                  sizeof(objects) / sizeof(objects[0]));
        slub_validate();

        for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i += 2) {
            slub_free(objects[i]);
        }
        for (size_t i = 1; i < sizeof(sizes) / sizeof(sizes[0]); i += 2) {
            slub_test_check(objects[i], sizes[i],
                            (uint8_t)(0x10 + round * 23 + i));
        }
        slub_validate();

        for (size_t i = 1; i < sizeof(sizes) / sizeof(sizes[0]); i += 2) {
            slub_free(objects[i]);
        }
        assert(nr_free_pages() == before);
        slub_assert_empty();
    }

    cprintf("SLUB test 7 - state consistency: PASS\n");
}

void slub_check(void) {
    assert(slub_initialized);
    size_t before = nr_free_pages();
    slub_assert_empty();

    slub_test_size_classes();
    slub_test_data_integrity();
    slub_test_object_reuse();
    slub_test_slab_expansion();
    slub_test_page_reclaim();
    slub_test_irregular_sizes();
    slub_test_state_consistency();

    assert(nr_free_pages() == before);
    slub_assert_empty();
    cprintf("SLUB tests: PASS\n");
}
