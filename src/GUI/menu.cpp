#include "menu.h"

#include <math.h>

#include "text_builder.h"

Menu::Menu(unsigned int _x, unsigned int _y, unsigned int _height, 
    unsigned int _width, uint32_t _color) 
        : gui(_height, _width, 
            (uint32_t *)psp_malloc(_width * _height * sizeof(uint32_t)), "menu"
) {
    x = _x;
    y = _y;
    height = _height;
    width = _width;
    background_color = _color;
}

Menu::Menu(Position _pos, unsigned int _height, unsigned int _width, 
    uint32_t _color, int padding_x/* = 0*/, int padding_y/* = 0*/)
        : gui(_height, _width, 
            (uint32_t *)psp_malloc(_width * _height * sizeof(uint32_t)), "menu"
){
    Vector2d vec = pivot_to_coord(_pos, _height, _width, SCREEN_HEIGHT, 
        SCREEN_WIDTH_RES, true, padding_x, padding_y);

    x = vec.x;
    y = vec.y;
    height = _height;
    width = _width;
    background_color = _color;    
}

Menu::~Menu(){
//TODO DFS style dealloc
}

inline uint32_t Menu::highlight_selection(Component comp, uint32_t pixel){
    return pixel + comp.selected * selected_color;
}

Vector2d Menu::pivot_to_coord(Position pos, unsigned int height_obj, 
    unsigned int width_obj, unsigned int height_pan, unsigned int width_pan,
    bool screen_coord, int padding_x/* = 0*/, int padding_y/* = 0*/
){
    int menu_cen_x = width_pan/2;
    int menu_cen_y = height_pan/2;

    int _x,_y;

    switch (pos) {
    case CENTER:
        _x = menu_cen_x - width_obj/2 + padding_x;
        _y = menu_cen_y - height_obj/2 + padding_y;
        break;
    case CENTER_LEFT:
        _x = 0 + padding_x;
        _y = menu_cen_y - height_obj/2 + padding_y;
        break;
    case CENTER_RIGHT:
        _x = width_pan - width_obj + padding_x;
        _y = menu_cen_y - height_obj/2 + padding_y;
        break;
    case TOP_CENTER:
        _x = menu_cen_x - width_obj/2 + padding_x;
        _y = (screen_coord) ? 0 + padding_y : height_pan - height_obj + padding_y;
        break;
    case BOTTOM_CENTER:
        _x = menu_cen_x - width_obj/2 + padding_x;
        _y = (screen_coord) ? height_pan - height_obj + padding_y: 0 + padding_y;
        break;
    case TOP_LEFT:
        _x = 0 + padding_x;
        _y = (screen_coord) ? 0 + padding_y : height_pan - height_obj + padding_y;
        break;
    case TOP_RIGHT:
        _x = width_pan - width_obj + padding_x;
        _y = (screen_coord) ? 0 + padding_y : height_pan - height_obj + padding_y;
        break;
    case BOTTOM_LEFT:
        _x = 0 + padding_x;
        _y = (screen_coord) ? height_pan - height_obj + padding_y: 0 + padding_y;
        break;
    case BOTTOM_RIGHT:
        _x = width_pan - width_obj + padding_x;
        _y = (screen_coord) ? height_pan - height_obj + padding_y: 0 + padding_y;
        break;
    \
    default:
        _x = _y = 0;
        log(CRITICAL, "Failed to match a pivot");
        break;
    }

    return Vector2d(_x,_y);
}

std::pair<int,int> Menu::add_component(Position _pos, Component comp, int padding_x /* = 0*/, 
    int padding_y /* = 0*/
){
    int comp_index = components.size();
    int group_index = groups.size();

    Vector2d vec = pivot_to_coord(_pos, comp.height, comp.width, height, width, 
        false, padding_x, padding_y);
    comp.x = vec.x;
    comp.y = vec.y;

    assign_comp_id(comp);
    assign_group_id(comp);
    components.push_back(comp);
    groups.push_back(GroupInfo(comp.group_ID, true, 1, 1, 
        {&components[components.size()-1]}));

    return {comp_index, group_index};
}

std::pair<int,int> Menu::add_component_group(Position pos, std::vector<Component> arr, 
    Grouping grouping, int spacing/* = 1*/, int padding_x/* = 0*/, int padding_y/* = 0*/,
    int rows/* = 0*/, int cols/* = 0*/, bool row_major/* = true*/
) {
    int first_comp_index = components.size();
    int group_index = groups.size();
    switch (grouping)
    {
    case VERTICAL_LIST:
        rows = arr.size();
        cols = 1;
        add_grid_row_major(pos, arr, spacing, padding_x, padding_y, arr.size(), 1);
        break;

    case HORIZONTAL_LIST:
        row_major = false;
        rows = 1;
        cols = arr.size();
        add_grid_row_major(pos, arr, spacing, padding_x, padding_y, 1, arr.size());
        break;
    
    case GRID:
        if(row_major) add_grid_row_major(pos, arr, spacing, padding_x,
            padding_y, rows, cols);
        else add_grid_col_major(pos, arr, spacing, padding_x, padding_y, 
            rows, cols);
        break;
    
    default:
        assert(0, "Failed to match grouping %d", grouping);
        break;
    }

    std::vector<Component *> group_vec;
    for(int i = components.size() - arr.size() - 1; i < components.size(); i++) {
        group_vec.push_back(&components[i]);
    }
    groups.push_back(GroupInfo(groups.size(), row_major, rows, cols, group_vec));

    return {first_comp_index, group_index};
}

void Menu::add_grid_row_major(Position pos, std::vector<Component> arr,
    int spacing/* = 1*/, int padding_x/* = 0*/, int padding_y/* = 0*/,
    int rows/* = 0*/, int cols/* = 0*/
) {
    int widest[cols] = {0};
    int tallest[rows] = {0};

    int total_height = 0;
    int total_width = 0;
    
    for (unsigned int i = 0; i < arr.size(); i++) {
        if (arr[i].width > widest[i%cols]) widest[i%cols] = arr[i].width;
        if (arr[i].height > tallest[i/cols]) tallest[i/cols] = arr[i].height;
    }

    for (int i=0; i < cols; i++) total_width += (widest[i] + spacing);
    for (int i=0; i < rows; i++) total_height += (tallest[i] + spacing);
    total_height -= spacing;
    total_width -= spacing;

    Vector2d curr_coord = pivot_to_coord(pos, total_height, total_width, height, width, false);
    curr_coord.x += padding_x;
    curr_coord.y += padding_y;

    curr_coord.y += total_height - tallest[0]; // top down left right
    int start_x = curr_coord.x;

    assign_group_id(arr);
    for (unsigned int i = 0; i < arr.size(); i++) {
        if (i % cols == 0 && i > 0) { // next row
            curr_coord.y -= tallest[i/cols] + spacing;
            curr_coord.x = start_x;
        }

        arr[i].x = curr_coord.x;
        arr[i].y = curr_coord.y;

        arr[i].y += (tallest[i/cols] - arr[i].height)/2;
        arr[i].x += (widest[i%cols] - arr[i].width)/2;
        
        curr_coord.x += widest[i%cols] + spacing;

        assign_comp_id(arr[i]);
        components.push_back(arr[i]);
    }
}

void Menu::add_grid_col_major(Position pos, std::vector<Component> arr, 
    int spacing/* = 1*/, int padding_x/* = 0*/, int padding_y/* = 0*/,
    int rows/* = 0*/, int cols/* = 0*/
) {
    int widest[rows] = {0};
    int tallest[cols] = {0};

    int total_height = 0;
    int total_width = 0;
    
    for (unsigned int i = 0; i < arr.size(); i++) {
        if (arr[i].width > widest[i/rows]) widest[i/rows] = arr[i].width;
        if (arr[i].height > tallest[i%rows]) tallest[i%rows] = arr[i].height;
    }

    for (int i=0; i < cols; i++) total_width += (widest[i] + spacing);
    for (int i=0; i < rows; i++) total_height += (tallest[i] + spacing);
    total_height -= spacing;
    total_width -= spacing;

    Vector2d curr_coord = pivot_to_coord(pos, total_height, total_width, height, width, false);
    curr_coord.x += padding_x;
    curr_coord.y += padding_y;

    curr_coord.y += total_height - tallest[0]; // top down left right
    int start_y = curr_coord.y;
    assign_group_id(arr);
    for (unsigned int i = 0; i < arr.size(); i++) {
        if (i % rows == 0 && i > 0) { // next row
            curr_coord.x += widest[i%rows] + spacing;
            curr_coord.y = start_y;
        }

        arr[i].x = curr_coord.x;
        arr[i].y = curr_coord.y;

        arr[i].y -= (tallest[i%rows] - arr[i].height)/2;
        arr[i].x += (widest[i/rows] - arr[i].width)/2;
        
        curr_coord.y -= tallest[i/rows] + spacing;

        assign_comp_id(arr[i]);
        components.push_back(arr[i]);
    }
}

void Menu::draw_panel(Component comp){
    switch (comp.data.data.shape)
    {
    case Component::Rectangle:
        for (int y = comp.y; y < comp.height + comp.y; y++){
            for (int x = comp.x; x < comp.width + comp.x; x++) {
                gui.img_matrix[width*y + x] = highlight_selection(comp, comp.background_color);
            }
        }
        break;

    case Component::Circle:
        int _x;
        int _y;
        for (int y = comp.y; y < comp.height + comp.y; y++){
            for (int x = comp.x; x < comp.width + comp.x; x++) {
                _x = comp.x - x + comp.width/2;
                _y = comp.y - y + comp.height/2;

                // We know that the general equation for a circle is 
                // ( x - h )^2 + ( y - k )^2 = r^2, where ( h, k ) is the center 
                // and r is the radius.
                if((pow(_x,2) + pow(_y,2)) <= pow(comp.width/2,2)) {
                    gui.img_matrix[width*y + x] = highlight_selection(comp, comp.background_color);  
                } 
            }
        }
        break;
    
    default:
        break;
    }
}

void Menu::draw_img(Component comp){
    Image _img = comp.data.data.img;
    for (unsigned int y = comp.y; y < _img.height + comp.y; y++){
        for (unsigned int x = comp.x; x < _img.width + comp.x; x++) {
            if (y < height && x < width) {
                if (GFX::is_transparent(_img.img_matrix[_img.width*(y - comp.y) + (x - comp.x)])){
                    if (comp.background_color ){
                        gui.img_matrix[width*y + x] = highlight_selection(comp, comp.background_color);
                    }
                }
                else {
                    gui.img_matrix[width*y + x] = highlight_selection(comp, _img.img_matrix[_img.width*(y - comp.y) + (x - comp.x)]);
                }
            }
        }
    }
}

void Menu::set_pos(Position pos, int padding_x /* = 0*/, int padding_y /* = 0*/){
    Vector2d vec  = pivot_to_coord(pos, height, width, SCREEN_HEIGHT, 
        SCREEN_WIDTH_RES, true, padding_x, padding_y);

    x = vec.x;
    y = vec.y;
}

void Menu::update(){   
    for (unsigned int i = 0; i < width * height; i++) gui.img_matrix[i] = background_color; 
    for (Component comp: components){
        if (comp.hidden) continue;

        if (comp.data.type == Component::IMAGE_TYPE) {
            draw_img(comp);
        }
        if (comp.data.type == Component::LABEL_TYPE) {
            //TODO simplify this
            Component temp = comp;
            temp.data.type = Component::IMAGE_TYPE;
            temp.data.data.img = text(comp.data.data.text); // Todo: data.data?
            draw_img(temp);
            psp_free(temp.data.data.img.img_matrix);
        }
        if (comp.data.type == Component::PANEL_TYPE) {
            draw_panel(comp);
        }
    }
}

// TODO remove
std::vector<int> Menu::get_selectable_components(){
    std::vector<int> selectable_arr;
    
    for (unsigned int i = 0; i < components.size(); i++){
        if (components[i].selectable) selectable_arr.push_back(i);
    }

    return selectable_arr;
}

std::vector<int> Menu::get_selectable_components(std::vector<Component *> arr){
    std::vector<int> selectable_arr;
    
    for (unsigned int i = 0; i < arr.size(); i++){
        if (arr[i]->selectable) selectable_arr.push_back(i);
    }

    return selectable_arr;
}

void Menu::select_component(int comp_id) {
    if(selected_comp_id > -1) components[selected_comp_id].deselect();

    components[selected_comp_id].select();
}

int Menu::find_component_by_id(int component_id){
    int index = -1;
    for (unsigned int i = 0; i < components.size(); i++){
        if (components[i].comp_ID == component_id) index = i;
    }
    return index;
}

void Menu::assign_comp_id(Component &comp){
    comp.comp_ID = next_id;
    next_id++;
}

void Menu::assign_group_id(Component &comp){
    comp.group_ID = next_group_id;
    next_group_id++;
}

void Menu::assign_group_id(std::vector<Component> &arr){
    for (Component &comp : arr) comp.group_ID = next_group_id;
    
    next_group_id++;
}

void Menu::select_next(){
    std::vector<int> selectable = get_selectable_components(groups[1].components);
    int old_index = selectable[selected_comp_id];
    components[old_index].deselect();
    selected_comp_id++;
    selected_comp_id %= selectable.size();
    log(DEBUG, "down %d, %u", selected_comp_id), selectable.size();
    int new_index = selectable[selected_comp_id];
    components[new_index].select();
}

void Menu::select_prev(){
    std::vector<int> selectable = get_selectable_components(groups[1].components);
    int old_index = selectable[selected_comp_id];
    components[old_index].deselect();
    (selected_comp_id)--;
    selected_comp_id = (selected_comp_id < 0) ? selectable.size() - 1: selected_comp_id;
    log(DEBUG, "up %d", selected_comp_id);
    int new_index = selectable[selected_comp_id];
    components[new_index].select();
}