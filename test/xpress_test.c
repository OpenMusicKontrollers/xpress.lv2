#include <assert.h>

#include <xpress.lv2/xpress.h>

#define MAX_NVOICES 32
#define MAX_URIDS 512

typedef struct _targetI_t targetI_t;
typedef struct _plughandle_t plughandle_t;
typedef struct _urid_t urid_t;

struct _targetI_t {
	int64_t frames;
	xpress_state_t state;
	xpress_uuid_t uuid;
};

struct _plughandle_t {
	XPRESS_T(xpressI, MAX_NVOICES);
	targetI_t targetI [MAX_NVOICES];
};

static void
_add(void *data, int64_t frames, const xpress_state_t *state,
	xpress_uuid_t uuid, void *target)
{
	plughandle_t *handle = data;
	targetI_t *src = target;

	(void)handle; //FIXME
	src->frames = frames;
	src->state = *state;
	src->uuid = uuid;
}

static void
_set(void *data, int64_t frames, const xpress_state_t *state,
	xpress_uuid_t uuid, void *target)
{
	plughandle_t *handle = data;
	targetI_t *src = target;

	(void)handle; //FIXME
	src->frames = frames;
	src->state = *state;
	assert(src->uuid == uuid);
}

static void
_del(void *data, int64_t frames,
	xpress_uuid_t uuid, void *target)
{
	plughandle_t *handle = data;
	targetI_t *src = target;

	(void)handle; //FIXME
	src->frames = frames;
	assert(src->uuid == uuid);
}

static const xpress_iface_t ifaceI = {
	.size = sizeof(targetI_t),
	.add = _add,
	.set = _set,
	.del = _del
};

struct _urid_t {
	LV2_URID urid;
	char *uri;
};

static urid_t urids [MAX_URIDS];
static LV2_URID urid;

static LV2_URID
_map(LV2_URID_Map_Handle instance __attribute__((unused)), const char *uri)
{
	urid_t *itm;
	for(itm=urids; itm->urid; itm++)
	{
		if(!strcmp(itm->uri, uri))
			return itm->urid;
	}

	assert(urid + 1 < MAX_URIDS);

	// create new
	itm->urid = ++urid;
	itm->uri = strdup(uri);

	return itm->urid;
}

static LV2_URID_Map map = {
	.handle = NULL,
	.map = _map
};

static xpress_uuid_t counter = 1;

static xpress_uuid_t
_new_uuid(void *handle __attribute__((unused)),
	uint32_t flag __attribute__((unused)))
{
	return counter++;
}

static xpress_map_t voice_map = {
	.handle = NULL,
	.new_uuid = _new_uuid
};

int
main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	static plughandle_t handle;

	assert(xpress_init(&handle.xpressI, MAX_NVOICES, &map, &voice_map,
			XPRESS_EVENT_ALL, &ifaceI, handle.targetI, &handle) == 1);

	//FIXME run tests here
	
	xpress_deinit(&handle.xpressI);

	xpress_uuid_t uuid = 0;
	targetI_t *src = xpress_create(&handle.xpressI, &uuid);
	assert(uuid != 0);
	assert(src);

	assert(xpress_get(&handle.xpressI, uuid) == src);
	assert(xpress_free(&handle.xpressI, uuid) == 1);
	assert(xpress_get(&handle.xpressI, uuid) == NULL);
	assert(xpress_free(&handle.xpressI, uuid) == 0);

	for(unsigned i=0; i<urid; i++)
	{
		urid_t *itm = &urids[i];

		free(itm->uri);
	}

	return 0;
}
