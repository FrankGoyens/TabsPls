import TabsPlsToolbar

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
