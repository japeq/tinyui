/*
 * gtk+ 2 implementation of tinyui GUI classes
 */
#include "tiny_ui.h"
#include <string.h>
#include <stdexcept>
#ifdef TINYUI_HILDON
#include <hildon/hildon.h>
#endif

namespace tinyui {

void Widget::show()
{
	gtk_widget_show(gtk_widget());
}

void Widget::hide()
{
	gtk_widget_hide(gtk_widget());
}

BoxLayout::BoxLayout(Orientation orientation) :
	m_orientation(orientation)
{
	m_expandable[HORIZONTAL] = false;
	m_expandable[VERTICAL] = false;
	switch (orientation) {
	case HORIZONTAL:
		m_gtkwidget = gtk_hbox_new(false, 3);
		break;
	case VERTICAL:
		m_gtkwidget = gtk_vbox_new(false, 3);
		break;
	default:
		throw std::runtime_error("Invalid orientation");
	}
}

BoxLayout::~BoxLayout()
{
	gtk_widget_destroy(m_gtkwidget);
}

void BoxLayout::add_widget(Widget *widget)
{
	if (widget->expandable(HORIZONTAL))
		m_expandable[HORIZONTAL] = true;
	if (widget->expandable(VERTICAL))
		m_expandable[VERTICAL] = true;
	gtk_box_pack_start(GTK_BOX(m_gtkwidget), widget->gtk_widget(),
			   widget->expandable(m_orientation), true, 0);
}

bool BoxLayout::expandable(Orientation orientation)
{
	return m_expandable[orientation];
}

GtkWidget *BoxLayout::gtk_widget()
{
	return m_gtkwidget;
}

Button::Button(const std::wstring &label) :
	m_handler(NULL)
{
	m_gtkwidget = gtk_button_new_with_label(encode_utf8(label).c_str());
	g_signal_connect(m_gtkwidget, "clicked", G_CALLBACK(clicked_cb), this);
}

Button::~Button()
{
	gtk_widget_destroy(m_gtkwidget);
}

void Button::set_label(const std::wstring &label)
{
	gtk_button_set_label(GTK_BUTTON(m_gtkwidget),
			     encode_utf8(label).c_str());
}

bool Button::expandable(Orientation orientation)
{
	return orientation == HORIZONTAL;
}

GtkWidget *Button::gtk_widget()
{
	return m_gtkwidget;
}

void Button::clicked_cb(GtkWidget *widget, Button *button)
{
	UNUSED(widget);
	if (button->m_handler)
		button->m_handler->clicked(button);
}

ListBoxItem::ListBoxItem(const std::wstring &text) :
	m_text(text), m_rowref(NULL)
{
}

ListBoxItem::~ListBoxItem()
{
	if (m_rowref == NULL)
		return;

	GtkTreeModel *model = gtk_tree_row_reference_get_model(m_rowref);
	GtkTreePath *path = gtk_tree_row_reference_get_path(m_rowref);
	GtkTreeIter iter;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);

	gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	gtk_tree_row_reference_free(m_rowref);
}

void ListBoxItem::set_text(const std::wstring &text)
{
	m_text = text;
	if (m_rowref == NULL)
		return;

	GtkTreeModel *model = gtk_tree_row_reference_get_model(m_rowref);
	GtkTreePath *path = gtk_tree_row_reference_get_path(m_rowref);
	GtkTreeIter iter;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);

	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			   0, encode_utf8(text).c_str(), -1);
}

ListBox::ListBox() :
	m_handler(NULL)
{
	m_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
	m_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(m_store));
	g_signal_connect(m_treeview, "row-activated", G_CALLBACK(activated_cb), this);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(m_treeview), false);

	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(m_treeview),
		-1, "", renderer, "text", 0, NULL);

#ifdef TINYUI_HILDON
	m_gtkwidget = hildon_pannable_area_new();
#else
	m_gtkwidget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_gtkwidget),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
#endif
	gtk_container_add(GTK_CONTAINER(m_gtkwidget), m_treeview);
}

ListBox::~ListBox()
{
	gtk_widget_destroy(m_gtkwidget);
}

void ListBox::add_item(ListBoxItem *item)
{
	GtkTreeIter iter;
	gtk_list_store_append(m_store, &iter);
	gtk_list_store_set(m_store, &iter,
			   0, encode_utf8(item->m_text).c_str(), 1, item, -1);

	GtkTreePath *path =
		gtk_tree_model_get_path(GTK_TREE_MODEL(m_store), &iter);
	item->m_rowref = gtk_tree_row_reference_new(GTK_TREE_MODEL(m_store), path);
	gtk_tree_path_free(path);
}

void ListBox::scroll_to(ListBoxItem *item)
{
	GtkTreePath *path = gtk_tree_row_reference_get_path(item->m_rowref);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(m_treeview), path, NULL,
				     false, 0, 0);
	gtk_tree_path_free(path);
}

void ListBox::activated_cb(GtkTreeView *treeview, GtkTreePath *path,
			   GtkTreeViewColumn *col, ListBox *listbox)
{
	UNUSED(treeview);
	UNUSED(path);
	UNUSED(col);
	GtkTreeIter iter;
	gtk_tree_model_get_iter(GTK_TREE_MODEL(listbox->m_store), &iter, path);

	GValue val;
	memset(&val, 0, sizeof val);
	gtk_tree_model_get_value(GTK_TREE_MODEL(listbox->m_store), &iter, 1, &val);
	ListBoxItem *item =
		reinterpret_cast<ListBoxItem *>(g_value_get_pointer(&val));
	g_value_unset(&val);

	if (listbox->m_handler)
		listbox->m_handler->clicked(listbox, item);
}

bool ListBox::expandable(Orientation orientation)
{
	UNUSED(orientation);
	return true;
}

GtkWidget *ListBox::gtk_widget()
{
	return m_gtkwidget;
}

Window::Window(const std::wstring &title)
{
	m_gtkwidget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(m_gtkwidget), 5);
	gtk_window_set_title(GTK_WINDOW(m_gtkwidget), encode_utf8(title).c_str());
}

Window::~Window()
{
	gtk_widget_destroy(m_gtkwidget);
}

void Window::set_widget(Widget *widget)
{
	gtk_container_add(GTK_CONTAINER(m_gtkwidget), widget->gtk_widget());
}

void Window::show()
{
	gtk_widget_show_all(m_gtkwidget);
}

Entry::Entry(const std::wstring &text) :
	m_handler(NULL)
{
	m_gtkwidget = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(m_gtkwidget), encode_utf8(text).c_str());
	g_signal_connect(m_gtkwidget, "activate", G_CALLBACK(activate_cb), this);
}

Entry::~Entry()
{
	gtk_widget_destroy(m_gtkwidget);
}

void Entry::set_text(const std::wstring &text)
{
	gtk_entry_set_text(GTK_ENTRY(m_gtkwidget), encode_utf8(text).c_str());
}

std::wstring Entry::get_text() const
{
	return decode_utf8(gtk_entry_get_text(GTK_ENTRY(m_gtkwidget)));
}

bool Entry::expandable(Orientation orientation)
{
	return orientation == HORIZONTAL;
}

GtkWidget *Entry::gtk_widget()
{
	return m_gtkwidget;
}

void Entry::activate_cb(GtkWidget *widget, Entry *entry)
{
	UNUSED(widget);
	if (entry->m_handler)
		entry->m_handler->activated(entry);
}

IoWatch::IoWatch(int fd, IoDirection dir) :
	m_handler(NULL)
{
	m_iochannel = g_io_channel_unix_new(fd);
	int cond = 0;
	if (dir & IN)
		cond |= G_IO_IN;
	if (dir & OUT)
		cond |= G_IO_OUT;
	m_id = g_io_add_watch(m_iochannel, GIOCondition(cond),
			      GIOFunc(io_watch_cb), this);
}

IoWatch::~IoWatch()
{
	g_source_remove(m_id);
	g_io_channel_unref(m_iochannel);
}

bool IoWatch::io_watch_cb(GIOChannel *iochannel, GIOCondition cond,
			  IoWatch *iowatch)
{
	int dir = 0;
	if (cond & G_IO_IN)
		dir |= IN;
	if (cond & G_IO_OUT)
		dir |= OUT;
	UNUSED(iochannel);
	if (iowatch->m_handler)
		iowatch->m_handler->ready(iowatch, IoDirection(dir));
	return true;
}

Timer::Timer(int interval) :
	m_handler(NULL)
{
	m_id = g_timeout_add(interval, GSourceFunc(timer_cb), this);
}

Timer::~Timer()
{
	g_source_remove(m_id);
}

bool Timer::timer_cb(Timer *timer)
{
	if (timer->m_handler)
		timer->m_handler->timeout(timer);
	return true;
}

Application::Application(int *argc, char ***argv)
{
	if (m_instance)
		throw std::runtime_error("Application instance already created");
	m_instance = this;
	gtk_init(argc, argv);
}

void Application::quit()
{
	gtk_main_quit();
}

int Application::run()
{
	gtk_main();
	return 0;
}

}
