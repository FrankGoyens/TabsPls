import TabsPlsToolbar

items = {
    "abc": {"display_name": "/home/frank"}, 
    "def": {"display_name": "/opt"}
}

def get_items(**kwargs):
    return items

def activate(item, activation_method):
    if item.id not in items:
        return None
    if activation_method == TabsPlsToolbar.ACTIVATION_METHOD_TYPES.regular:
        return TabsPlsToolbar.ACTIVATION_RESULT_ACTIONS.change_dir, item[item.id].display_name
    if activation_method == TabsPlsToolbar.ACTIVATION_METHOD_TYPES.alternative:
        return TabsPlsToolbar.ACTIVATION_RESULT_ACTIONS.open_dir_in_tab, item[item.id].display_name
    
