#include <buddy_pmm.h>
#include <list.h>
#include <pmm.h>
#include <stdio.h>

// lab2 buddy system 2413995

#define BUDDY_MAX_ORDER 31

static list_entry_t buddy_free_lists[BUDDY_MAX_ORDER + 1];
static size_t buddy_nr_free;

static size_t buddy_order_size(unsigned int order) {
    return (size_t)1 << order;
}

static int buddy_order_for_size(size_t n, size_t *rounded_size) {
    size_t size = 1;
    unsigned int order = 0;

    while (size < n) {
        if (order == BUDDY_MAX_ORDER || size > ((size_t)-1 >> 1)) {
            return -1;
        }
        size <<= 1;
        order++;
    }
    *rounded_size = size;
    return (int)order;
}

static void buddy_insert_sorted(struct Page *base, unsigned int order) {
    list_entry_t *head = &buddy_free_lists[order];
    list_entry_t *le = head;

    while ((le = list_next(le)) != head) {
        struct Page *page = le2page(le, page_link);
        if (base < page) {
            list_add_before(le, &base->page_link);
            return;
        }
    }
    list_add_before(head, &base->page_link);
}

static bool buddy_page_range_valid(ppn_t ppn, size_t block_size) {
    return ppn >= nbase && ppn < npage && block_size <= npage - ppn;
}

static void buddy_insert_and_merge(struct Page *base, unsigned int order) {
    size_t block_size = buddy_order_size(order);

    base->property = (unsigned int)block_size;
    SetPageProperty(base);

    while (order < BUDDY_MAX_ORDER) {
        ppn_t base_ppn = page2ppn(base);
        ppn_t buddy_ppn = base_ppn ^ block_size;
        if (!buddy_page_range_valid(buddy_ppn, block_size)) {
            break;
        }

        struct Page *other = &pages[buddy_ppn - nbase];
        if (!PageProperty(other) || other->property != block_size) {
            break;
        }

        list_del(&other->page_link);
        ClearPageProperty(other);
        other->property = 0;

        if (other < base) {
            ClearPageProperty(base);
            base->property = 0;
            base = other;
        }

        order++;
        block_size <<= 1;
        base->property = (unsigned int)block_size;
        SetPageProperty(base);
    }

    buddy_insert_sorted(base, order);
}

static void buddy_init(void) {
    for (unsigned int order = 0; order <= BUDDY_MAX_ORDER; order++) {
        list_init(&buddy_free_lists[order]);
    }
    buddy_nr_free = 0;
}

static void buddy_init_memmap(struct Page *base, size_t n) {
    assert(n > 0);
    assert(base >= pages);
    assert((size_t)(base - pages) <= npage - nbase);
    assert(n <= npage - nbase - (size_t)(base - pages));

    for (struct Page *page = base; page != base + n; page++) {
        assert(PageReserved(page));
        page->flags = 0;
        page->property = 0;
        set_page_ref(page, 0);
        list_init(&page->page_link);
    }

    struct Page *current = base;
    size_t remaining = n;
    while (remaining > 0) {
        ppn_t ppn = page2ppn(current);
        unsigned int order = 0;

        while (order < BUDDY_MAX_ORDER) {
            size_t next_size = buddy_order_size(order + 1);
            if (next_size > remaining || (ppn & (next_size - 1)) != 0) {
                break;
            }
            order++;
        }

        size_t block_size = buddy_order_size(order);
        buddy_insert_and_merge(current, order);
        current += block_size;
        remaining -= block_size;
    }

    buddy_nr_free += n;
}

static struct Page *buddy_alloc_pages(size_t n) {
    assert(n > 0);
    if (n > buddy_nr_free) {
        return NULL;
    }

    size_t block_size;
    int target = buddy_order_for_size(n, &block_size);
    if (target < 0 || block_size > buddy_nr_free) {
        return NULL;
    }

    unsigned int order = (unsigned int)target;
    unsigned int found = order;
    while (found <= BUDDY_MAX_ORDER &&
           list_empty(&buddy_free_lists[found])) {
        found++;
    }
    if (found > BUDDY_MAX_ORDER) {
        return NULL;
    }

    list_entry_t *entry = list_next(&buddy_free_lists[found]);
    struct Page *base = le2page(entry, page_link);
    list_del(entry);
    ClearPageProperty(base);
    base->property = 0;

    while (found > order) {
        found--;
        size_t half_size = buddy_order_size(found);
        struct Page *right = base + half_size;
        right->property = (unsigned int)half_size;
        SetPageProperty(right);
        buddy_insert_sorted(right, found);
    }

    base->property = (unsigned int)block_size;
    ClearPageProperty(base);
    buddy_nr_free -= block_size;
    return base;
}

static void buddy_free_pages(struct Page *base, size_t n) {
    assert(n > 0);

    size_t block_size;
    int result = buddy_order_for_size(n, &block_size);
    assert(result >= 0);

    assert(base >= pages);
    size_t index = (size_t)(base - pages);
    assert(index < npage - nbase);
    assert(block_size <= npage - nbase - index);
    assert(!PageReserved(base) && !PageProperty(base));
    assert(base->property == block_size);

    for (struct Page *page = base; page != base + block_size; page++) {
        assert(!PageReserved(page) && !PageProperty(page));
        page->flags = 0;
        page->property = 0;
        set_page_ref(page, 0);
        list_init(&page->page_link);
    }

    buddy_nr_free += block_size;
    buddy_insert_and_merge(base, (unsigned int)result);
}

static size_t buddy_nr_free_pages(void) {
    return buddy_nr_free;
}

static size_t buddy_validate(void) {
    size_t total = 0;

    for (unsigned int order = 0; order <= BUDDY_MAX_ORDER; order++) {
        size_t block_size = buddy_order_size(order);
        list_entry_t *head = &buddy_free_lists[order];
        list_entry_t *le = head;

        while ((le = list_next(le)) != head) {
            struct Page *base = le2page(le, page_link);
            ppn_t ppn = page2ppn(base);

            assert(PageProperty(base));
            assert(base->property == block_size);
            assert((ppn & (block_size - 1)) == 0);
            assert(buddy_page_range_valid(ppn, block_size));

            for (size_t i = 0; i < block_size; i++) {
                assert(!PageReserved(base + i));
            }

            ppn_t buddy_ppn = ppn ^ block_size;
            if (buddy_page_range_valid(buddy_ppn, block_size)) {
                struct Page *other = &pages[buddy_ppn - nbase];
                assert(!PageProperty(other) ||
                       other->property != block_size);
            }
            total += block_size;
        }
    }

    assert(total == buddy_nr_free);
    return total;
}

struct buddy_test_snapshot {
    list_entry_t free_lists[BUDDY_MAX_ORDER + 1];
    size_t nr_free;
};

static void buddy_test_detach(struct buddy_test_snapshot *snapshot) {
    for (unsigned int order = 0; order <= BUDDY_MAX_ORDER; order++) {
        snapshot->free_lists[order] = buddy_free_lists[order];
        list_init(&buddy_free_lists[order]);
    }
    snapshot->nr_free = buddy_nr_free;
    buddy_nr_free = 0;
}

static void buddy_test_reset_region(struct Page *arena, size_t arena_pages,
                                    struct Page *base, size_t n) {
    assert(base >= arena && n <= arena_pages);
    assert((size_t)(base - arena) <= arena_pages - n);

    for (unsigned int order = 0; order <= BUDDY_MAX_ORDER; order++) {
        list_init(&buddy_free_lists[order]);
    }
    buddy_nr_free = 0;

    for (size_t i = 0; i < arena_pages; i++) {
        arena[i].flags = 0;
        arena[i].property = 0;
        set_page_ref(arena + i, 0);
        list_init(&arena[i].page_link);
        SetPageReserved(arena + i);
    }

    buddy_init_memmap(base, n);
    assert(buddy_nr_free == n);
    assert(buddy_validate() == n);
}

static void buddy_test_restore(struct Page *arena, size_t arena_pages,
                               struct buddy_test_snapshot *snapshot,
                               size_t original_free) {
    for (size_t i = 0; i < arena_pages; i++) {
        arena[i].flags = 0;
        arena[i].property = 0;
        set_page_ref(arena + i, 0);
        list_init(&arena[i].page_link);
    }
    arena->property = (unsigned int)arena_pages;

    for (unsigned int order = 0; order <= BUDDY_MAX_ORDER; order++) {
        buddy_free_lists[order] = snapshot->free_lists[order];
    }
    buddy_nr_free = snapshot->nr_free;

    buddy_free_pages(arena, arena_pages);
    assert(buddy_nr_free == original_free);
    buddy_validate();
}

static bool buddy_test_overlap(struct Page *left, size_t left_size,
                               struct Page *right, size_t right_size) {
    return left < right + right_size && right < left + left_size;
}

static void buddy_test_assert_disjoint(struct Page **blocks, size_t count) {
    for (size_t i = 0; i < count; i++) {
        assert(blocks[i] != NULL);
        for (size_t j = i + 1; j < count; j++) {
            assert(!buddy_test_overlap(blocks[i], blocks[i]->property,
                                       blocks[j], blocks[j]->property));
        }
    }
}

static void buddy_test_basic(struct Page *arena) {
    buddy_test_reset_region(arena, 32, arena, 32);

    const size_t requests[] = {1, 2, 4, 8};
    struct Page *blocks[4];
    for (size_t i = 0; i < 4; i++) {
        blocks[i] = alloc_pages(requests[i]);
        assert(blocks[i] != NULL);
        assert(blocks[i]->property == requests[i]);
        assert(blocks[i] >= arena &&
               blocks[i] + blocks[i]->property <= arena + 32);
    }
    buddy_test_assert_disjoint(blocks, 4);
    assert(nr_free_pages() == 17);

    for (size_t i = 4; i > 0; i--) {
        free_pages(blocks[i - 1], requests[i - 1]);
    }
    assert(nr_free_pages() == 32);
    buddy_validate();
    cprintf("Buddy test 1 - basic allocation: PASS\n");
}

static void buddy_test_non_power_of_two(struct Page *arena) {
    buddy_test_reset_region(arena, 32, arena, 32);

    const size_t requests[] = {3, 5, 7};
    const size_t expected[] = {4, 8, 8};
    struct Page *blocks[3];
    for (size_t i = 0; i < 3; i++) {
        blocks[i] = alloc_pages(requests[i]);
        assert(blocks[i] != NULL);
        assert(blocks[i]->property == expected[i]);
    }
    buddy_test_assert_disjoint(blocks, 3);
    assert(nr_free_pages() == 12);

    for (size_t i = 0; i < 3; i++) {
        free_pages(blocks[i], requests[i]);
    }
    assert(nr_free_pages() == 32);
    buddy_validate();
    cprintf("Buddy test 2 - rounded requests: PASS\n");
}

static void buddy_test_split(struct Page *arena) {
    buddy_test_reset_region(arena, 32, arena, 32);

    struct Page *page = alloc_pages(1);
    assert(page == arena && page->property == 1);
    assert(PageProperty(arena + 1) && arena[1].property == 1);
    assert(PageProperty(arena + 2) && arena[2].property == 2);
    assert(PageProperty(arena + 4) && arena[4].property == 4);
    assert(PageProperty(arena + 8) && arena[8].property == 8);
    assert(PageProperty(arena + 16) && arena[16].property == 16);
    assert(nr_free_pages() == 31);
    buddy_validate();

    free_pages(page, 1);
    assert(PageProperty(arena) && arena->property == 32);
    assert(nr_free_pages() == 32);
    buddy_validate();
    cprintf("Buddy test 3 - recursive split: PASS\n");
}

static void buddy_test_merge(struct Page *arena) {
    buddy_test_reset_region(arena, 32, arena, 32);

    struct Page *blocks[4];
    for (size_t i = 0; i < 4; i++) {
        blocks[i] = alloc_pages(1);
        assert(blocks[i] == arena + i);
    }

    free_pages(blocks[1], 1);
    assert(PageProperty(arena + 1) && arena[1].property == 1);
    free_pages(blocks[2], 1);
    assert(PageProperty(arena + 1) && arena[1].property == 1);
    assert(PageProperty(arena + 2) && arena[2].property == 1);

    free_pages(blocks[0], 1);
    assert(PageProperty(arena) && arena->property == 2);
    assert(PageProperty(arena + 2) && arena[2].property == 1);

    free_pages(blocks[3], 1);
    assert(PageProperty(arena) && arena->property == 32);
    assert(!PageProperty(arena + 1));
    assert(!PageProperty(arena + 2));
    assert(!PageProperty(arena + 3));
    assert(nr_free_pages() == 32);
    buddy_validate();
    cprintf("Buddy test 4 - buddy merge: PASS\n");
}

static void buddy_test_out_of_memory(struct Page *arena) {
    buddy_test_reset_region(arena, 32, arena, 32);

    struct Page *whole = alloc_pages(32);
    assert(whole == arena && whole->property == 32);
    assert(nr_free_pages() == 0);

    size_t before_failure = nr_free_pages();
    assert(alloc_pages(1) == NULL);
    assert(nr_free_pages() == before_failure);

    free_pages(whole, 32);
    before_failure = nr_free_pages();
    assert(alloc_pages(33) == NULL);
    assert(nr_free_pages() == before_failure);
    buddy_validate();
    cprintf("Buddy test 5 - out of memory: PASS\n");
}

static void buddy_test_irregular_region(struct Page *arena) {
    buddy_test_reset_region(arena, 32, arena + 1, 29);

    assert(PageReserved(arena));
    assert(PageReserved(arena + 30));
    assert(PageReserved(arena + 31));
    assert(nr_free_pages() == 29);

    size_t before_failure = nr_free_pages();
    assert(alloc_pages(16) == NULL);
    assert(nr_free_pages() == before_failure);

    const size_t requests[] = {1, 3, 5};
    const size_t expected[] = {1, 4, 8};
    struct Page *blocks[3];
    for (size_t i = 0; i < 3; i++) {
        blocks[i] = alloc_pages(requests[i]);
        assert(blocks[i] != NULL);
        assert(blocks[i]->property == expected[i]);
        assert(blocks[i] >= arena + 1 &&
               blocks[i] + blocks[i]->property <= arena + 30);
    }
    buddy_test_assert_disjoint(blocks, 3);

    for (size_t i = 3; i > 0; i--) {
        free_pages(blocks[i - 1], requests[i - 1]);
    }
    assert(nr_free_pages() == 29);
    buddy_validate();
    cprintf("Buddy test 6 - irregular region: PASS\n");
}

static void buddy_test_consistency(struct Page *arena) {
    buddy_test_reset_region(arena, 32, arena, 32);

    const size_t requests[] = {1, 2, 3, 4, 5};
    const size_t release_order[] = {2, 4, 0, 3, 1};
    struct Page *blocks[5];

    for (size_t i = 0; i < 5; i++) {
        blocks[i] = alloc_pages(requests[i]);
        assert(blocks[i] != NULL);
        buddy_validate();
    }
    buddy_test_assert_disjoint(blocks, 5);
    assert(nr_free_pages() == 13);

    for (size_t i = 0; i < 5; i++) {
        size_t index = release_order[i];
        free_pages(blocks[index], requests[index]);
        buddy_validate();
    }

    assert(nr_free_pages() == 32);
    assert(PageProperty(arena) && arena->property == 32);
    cprintf("Buddy test 7 - state consistency: PASS\n");
}

void buddy_check(void) {
    size_t original_free = buddy_validate();
    assert(original_free >= 32);

    struct Page *arena = alloc_pages(32);
    assert(arena != NULL && arena->property == 32);
    assert(nr_free_pages() == original_free - 32);

    struct buddy_test_snapshot snapshot;
    buddy_test_detach(&snapshot);

    buddy_test_basic(arena);
    buddy_test_non_power_of_two(arena);
    buddy_test_split(arena);
    buddy_test_merge(arena);
    buddy_test_out_of_memory(arena);
    buddy_test_irregular_region(arena);
    buddy_test_consistency(arena);

    buddy_test_restore(arena, 32, &snapshot, original_free);
    cprintf("Buddy System tests: PASS\n");
}

const struct pmm_manager buddy_pmm_manager = {
    .name = "buddy_pmm_manager",
    .init = buddy_init,
    .init_memmap = buddy_init_memmap,
    .alloc_pages = buddy_alloc_pages,
    .free_pages = buddy_free_pages,
    .nr_free_pages = buddy_nr_free_pages,
    .check = buddy_check,
};
