# these are usually defined in a separate file TabsPlsToolbar.py which is then imported
# but that is not necessary for testing purposes,
# there shouldn't be more than one TabsPlsToolbar.py in the repository as this defines a protocol
class TabsPlsToolbar(object):
	ACTIVATION_METHOD_TYPES_REGULAR = "regular"
	ACTIVATION_METHOD_TYPES_ALTERNATIVE = "alternative"

	ACTIVATION_RESULT_ACTION_CHDIR = "change_dir"
	ACTIVATION_RESULT_ACTION_OPEN_DIR_IN_TAB = "open_dir_in_tab"
	ACTIVATION_RESULT_ACTION_MESSAGE = "message"
##

items = {
    "abc": {"display_name": "/home/user"},
    "def": {"display_name": "/opt"},
    "ghi": {"display_name": "hello button"},
}


def get_items(**kwargs):
    return items


def activate(item_id, activation_method):
    if item_id not in items:
        return None
    item = items[item_id]
    if item_id == "ghi":
        return (TabsPlsToolbar.ACTIVATION_RESULT_ACTION_MESSAGE, "hello")
    if activation_method == TabsPlsToolbar.ACTIVATION_METHOD_TYPES_REGULAR:
        return (
            TabsPlsToolbar.ACTIVATION_RESULT_ACTION_CHDIR,
            item["display_name"],
        )
    if activation_method == TabsPlsToolbar.ACTIVATION_METHOD_TYPES_ALTERNATIVE:
        return (
            TabsPlsToolbar.ACTIVATION_RESULT_ACTION_OPEN_DIR_IN_TAB,
            item["display_name"],
        )
    return None
