#include "../source/steam/steam_library.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void add(struct steam_wire_writer *w, unsigned id, int family)
{
    unsigned char buf[128];
    struct steam_wire_writer a = {buf, sizeof(buf), 0, 0};
    unsigned char owner[8] = {42};
    steam_wire_uint(&a, 1, id);
    steam_wire_bytes(&a, family ? 6 : 2, "Same title", 10);
    if (family) steam_wire_bytes(&a, 2, owner, sizeof(owner));
    assert(!a.failed);
    steam_wire_bytes(w, family ? 1 : 2, buf, a.size);
    assert(!w->failed);
}

static void run(int reverse)
{
    unsigned char body[512];
    struct steam_wire_writer w = {body, sizeof(body), 0, 0};
    struct steam_cm_message m = {.type=147,.result=1,.body=body};
    struct steam_library lib = {0};
    struct steam_game page[2];
    size_t total;
    steam_wire_uint(&w, 1, 2);
    add(&w, reverse ? 20 : 40, 0);
    add(&w, reverse ? 40 : 20, 0);
    m.body_size = w.size;
    assert(steam_library_read(&m, 99, &lib) == STEAM_LIBRARY_OK);
    assert(lib.games[0].app_id == 20 && lib.games[1].app_id == 40);
    w = (struct steam_wire_writer){body, sizeof(body), 0, 0};
    add(&w, reverse ? 10 : 30, 1);
    add(&w, reverse ? 30 : 10, 1);
    m.body_size = w.size;
    assert(steam_library_merge_family(&m, &lib) == STEAM_LIBRARY_OK);
    assert(steam_library_page(&lib, "Same", 0, page, 2, &total) == 2);
    assert(total == 4 && page[0].app_id == 10 && page[1].app_id == 20);
    assert(steam_library_page(&lib, "Same", 2, page, 2, &total) == 2);
    assert(total == 4 && page[0].app_id == 30 && page[1].app_id == 40);
    steam_library_clear(&lib);
}

int main(void)
{
    run(0);
    run(1);
    puts("Stable Steam library order and pagination passed");
    return 0;
}
