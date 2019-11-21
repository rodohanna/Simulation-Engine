#include "GUI.h"
#include "Events.h"
#include "Assets.h"
#include "Window.h"
#include "Physics.h"

Text::Text(EventBus *e, size_t font_index, std::string texture_key) : event_bus(e), font_index(font_index), texture_key(texture_key){};

void Text::set_text(std::string text)
{
    if (text == "")
    {
        this->text = "";
        this->dimensions = {};
        return;
    }
    if (this->text != text)
    {
        auto info = Assets::create_texture_from_text(
            this->event_bus->renderer,
            this->font_index,
            this->texture_key,
            text,
            {0xFF, 0xFF, 0xFF, 0xFF});
        this->text = text;
        this->texture_index = info.texture_index;
        this->dimensions = info.dimensions;
    }
}

void Text::update(double ts, size_t z_index)
{
    if (this->text == "")
    {
        return;
    }
    this->event_bus->publish_render_event(
        Events::createRenderTextureEvent(
            this->texture_index,
            this->position,
            &this->overflow_clip,
            1,
            z_index));
}

V2 get_position_from_anchors(Rect *camera, V2 &dimensions, Anchor &anchor_horizontal, Anchor &anchor_vertical)
{
    V2 anchored_position = {};
    if (anchor_horizontal.type == AnchorType::RIGHT)
    {
        anchored_position.x = camera->w - dimensions.x - anchor_horizontal.offset;
    }
    else if (anchor_horizontal.type == AnchorType::LEFT)
    {
        anchored_position.x = anchor_horizontal.offset;
    }
    else
    {
        anchored_position.x = ((camera->w - dimensions.x) / 2) + anchor_horizontal.offset;
    }

    if (anchor_vertical.type == AnchorType::TOP)
    {
        anchored_position.y = anchor_vertical.offset;
    }
    else if (anchor_vertical.type == AnchorType::BOTTOM)
    {
        anchored_position.y = camera->h - dimensions.y - anchor_vertical.offset;
    }
    else
    {
        anchored_position.y = ((camera->h - dimensions.y) / 2) + anchor_vertical.offset;
    }
    return anchored_position;
};

void UIPanel::update_rect()
{
    V2 anchored_position = get_position_from_anchors(Window::get_camera(), this->dimensions, this->anchor_horizontal, this->anchor_vertical);
    this->rect = {
        anchored_position.x,
        anchored_position.y,
        this->dimensions.x,
        this->dimensions.y};
    for (Text &t : this->panel_text)
    {
        t.overflow_clip = this->rect;
    }
};

void UIPanel::update(double ts)
{
    this->update_rect();
    int last_text_height = 0;
    for (Text &t : this->panel_text)
    {
        t.position = {
            this->rect.x + (this->rect.w - t.dimensions.x) / 2,
            this->rect.y + last_text_height + 10};
        last_text_height += t.dimensions.y;
        t.update(ts, this->z_index + 1);
    }
    this->event_bus->publish_render_event(
        Events::createRenderRectangleEvent(this->rect, {0x11, 0x11, 0x11, 0xF0}, true, this->z_index));
    this->event_bus->publish_render_event(
        Events::createRenderRectangleEvent(this->rect, {0xFF, 0xFF, 0xFF, 0xF0}, false, this->z_index + 1));
}

TextInput::TextInput(EventBus *e, std::string texture_key) : event_bus(e)
{
    e->subscribe_to_input_events(this);
    Text *text = new Text(e, 0, texture_key);
    this->text = std::unique_ptr<Text>(text);
    this->blink_interval_millis = .5;
    this->blink_interval_counter = 0;
    this->render_cursor = true;
    this->mouse_clicked = false;
    this->is_active = false;
    this->text_buffer = "";
};

TextInput::~TextInput()
{
    this->event_bus->unsubscribe_to_input_events(this);
};

void TextInput::handle_input_events(const InputEvent *input_events, size_t count)
{
    this->mouse_clicked = false;
    for (size_t i = 0; i < count; ++i)
    {
        InputEvent e = input_events[i];
        if (e.type == InputEventType::TEXT_INPUT && this->is_active)
        {
            this->text_buffer.append(e.data.text_input_event.text);
        }
        else if (e.type == InputEventType::KEY_DOWN && this->is_active)
        {
            if (e.data.key_event.key == KeyEventType::BACKSPACE_KEY)
            {

                if (this->text_buffer != "")
                {
                    this->text_buffer.pop_back();
                }
            }
            else if (e.data.key_event.key == KeyEventType::ENTER_KEY)
            {
                printf("Enter pressed\n");
            }
        }
        else if (e.type == InputEventType::MOUSE_CLICK)
        {
            this->mouse_clicked = e.data.mouse_click_event.button == MouseButton::MOUSE_BUTTON_LEFT;
        }
    }
}

void TextInput::update(double ts)
{
    V2 anchored_position = get_position_from_anchors(Window::get_camera(), this->dimensions, this->anchor_horizontal, this->anchor_vertical);
    Rect input_rect = {anchored_position.x, anchored_position.y, this->dimensions.x, this->dimensions.y};
    this->event_bus->publish_render_event(
        Events::createRenderRectangleEvent(input_rect, {0x11, 0x11, 0x11, 0xF0}, true, this->z_index));

    if (this->mouse_clicked)
    {
        bool was_active = this->is_active;
        this->is_active = Physics::checkPointInRect(Window::get_mouse_position(), &input_rect);
        InputEvent input_event;
        if (was_active && !this->is_active)
        {
            input_event.type = InputEventType::GUI_UNFOCUSED;
            this->event_bus->publish_input_event(input_event);
        }
        else if (!was_active && this->is_active)
        {
            input_event.type = InputEventType::GUI_FOCUSED;
            this->event_bus->publish_input_event(input_event);
            this->render_cursor = false; // Ensure cursor is rendered on click.
            this->blink_interval_counter = this->blink_interval_millis;
        }
    }

    this->blink_interval_counter += ts;
    if (this->blink_interval_counter > this->blink_interval_millis)
    {
        this->blink_interval_counter = 0;
        this->render_cursor = !this->render_cursor;
    }
    int cursor_height = static_cast<int>(input_rect.h * 0.5);
    this->text->overflow_clip = input_rect;
    this->text->set_text(this->text_buffer);
    Rect cursor_rect = {anchored_position.x + this->text->dimensions.x + 5, anchored_position.y + (input_rect.h / 4), 2, cursor_height};
    this->text->position = {anchored_position.x + 5, cursor_rect.y};
    this->text->update(ts, this->z_index + 1);
    if (this->is_active && this->render_cursor)
    {
        this->event_bus->publish_render_event(
            Events::createRenderRectangleEvent(cursor_rect, {0xFF, 0xFF, 0xFF, 0xF0}, true, this->z_index + 1));
    }
}

GUI::GUI(EventBus *e) : event_bus(e)
{
    this->inventory_panel.event_bus = e;
    this->inventory_panel.dimensions = {275, 175};
    this->inventory_panel.anchor_horizontal = {AnchorType::RIGHT, 10};
    this->inventory_panel.anchor_vertical = {AnchorType::BOTTOM, 10};
    this->inventory_panel.z_index = 4;
    Text t = Text(e, 0, "test-text");
    t.set_text("Demo UI Panel");
    this->inventory_panel.panel_text.push_back(t);
}

void GUI::update(double ts)
{
    this->inventory_panel.update(ts);
};