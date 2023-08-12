#include <stdio.h>
#include <string.h>
#include <wayland-client.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "shm.h"
#include "xdg-shell-client-protocol.h"

struct demo_state {
	struct wl_compositor* w_compositor;
    struct wl_shm *w_shm;
	struct xdg_wm_base* x_wm_base;

	struct wl_surface* w_surface;
	struct xdg_surface* x_surface;
	struct xdg_toplevel* x_toplevel;
};

static void
wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
    /* Sent by the compositor when it's no longer using this buffer */
    wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener wl_buffer_listener = {
    .release = wl_buffer_release,
};

static struct wl_buffer *draw_frame(struct demo_state *state)
{
    const int width = 640, height = 480;
    int stride = width * 4;
    int size = stride * height;

    int fd = allocate_shm_file(size);
    if (fd == -1) {
        return NULL;
    }

    uint32_t *data = mmap(NULL, size,
            PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        return NULL;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(state->w_shm, fd, size);
    struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0,
            width, height, stride, WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);

    /* Draw checkerboxed background */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if ((x + y / 8 * 8) % 16 < 8)
                data[y * width + x] = 0xFF666666;
            else
                data[y * width + x] = 0xFFEEEEEE;
        }
    }

    munmap(data, size);
	wl_buffer_add_listener(buffer, &wl_buffer_listener, NULL);
    return buffer;
}

static void xsurface_handle_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
	struct demo_state* state = data;
	xdg_surface_ack_configure(xdg_surface, serial);

	fprintf(stderr, "Configure...");    
	struct wl_buffer *buffer = draw_frame(state);
	wl_surface_attach(state->w_surface, buffer, 0, 0);
	wl_surface_commit(state->w_surface);
}

static const struct xdg_surface_listener x_surface_listener = {
	.configure = xsurface_handle_configure
};

static void registry_handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version)
{
	struct demo_state* state = data;
	if (0 == strcmp(interface, wl_compositor_interface.name)) {
		state->w_compositor = wl_registry_bind(registry, name, &wl_compositor_interface, version);
	} else if (0 == strcmp(interface, xdg_wm_base_interface.name)) {
		state->x_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, version);
	} else if (0 == strcmp(interface, wl_shm_interface.name)) {
		state->w_shm = wl_registry_bind(registry, name, &wl_shm_interface, version);	
	}
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_handle_global,
};

static void init(struct demo_state* state) {
	state->w_surface = wl_compositor_create_surface(state->w_compositor);
	state->x_surface = xdg_wm_base_get_xdg_surface(state->x_wm_base, state->w_surface);
	xdg_surface_add_listener(state->x_surface, &x_surface_listener, state);
	state->x_toplevel = xdg_surface_get_toplevel(state->x_surface);
	xdg_toplevel_set_title(state->x_toplevel, "My App!");
	wl_surface_commit(state->w_surface);
}

int init_wayland(int argc, char *argv[])
{
    struct demo_state state = { 0 };
    struct wl_display *display = wl_display_connect(NULL);
	struct wl_registry *registry = wl_display_get_registry(display);
   	wl_registry_add_listener(registry, &registry_listener, &state);
	wl_display_roundtrip(display);

	fprintf(stderr, "All globals gotten\n");
	init(&state);	

    while (wl_display_dispatch(display) != -1) {
        /* This space deliberately left blank */
    }

    fprintf(stderr, "Connection established!\n");
    wl_display_disconnect(display);
	wl_registry_destroy(registry);
    return 0;
}
