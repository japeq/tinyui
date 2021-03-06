#ifndef _TINY_UI_H
#define _TINY_UI_H

#ifdef TINYUI_GTK
#include <gtk/gtk.h>
#endif
#include <string>
#include <sstream>
#include <iomanip>

#ifdef TINYUI_QT
#include <QObject>

class QWidget;
class QPushButton;
class QBoxLayout;
class QWidget;
class QSocketNotifier;
class QListWidgetItem;
class QListWidget;
class QLineEdit;
#else
#define Q_OBJECT
class QObject {};
#endif

#define UNUSED(x) (void)(x)

#define DISABLE_ASSIGN(x) \
	private: \
		void operator =(const x &from);

#undef foreach
#define foreach(type, i, container) \
	for (type::iterator i = (container).begin(); i != (container).end(); ++i)

#define foreach_safe(type, i, container) \
	for (type::iterator next = (container).begin(), i = next++; \
		 i != (container).end(); i = next++)

namespace tinyui {

std::string encode_utf8(const std::wstring &in);
std::wstring decode_utf8(const std::string &in);

template<class T>
std::wstring to_wstring(const T &str)
{
	std::wstring out;
	out.assign(str.begin(), str.end());
	return out;
}

template<class T>
std::wstring format_number(T val)
{
	std::ostringstream str;
	str << val;
	return to_wstring(str.str());
}

template<class T>
std::wstring format_hex(T val, size_t width)
{
	std::ostringstream str;
	str << std::setw(width) << std::setfill('0') << std::hex << val;
	return to_wstring(str.str());
}

enum Orientation {
	VERTICAL,
	HORIZONTAL,
};

/* bitfield */
enum IoDirection {
	IN = 1,
	OUT = 2,
};

class Widget {
public:
	Widget() {}

	void show();
	void hide();

#ifdef TINYUI_GTK
	virtual bool expandable(Orientation orientation) = 0;
	virtual GtkWidget *gtk_widget() = 0;
#endif
#ifdef TINYUI_QT
	virtual QWidget *qt_widget() = 0;
#endif
};

class BoxLayout: public Widget {
	DISABLE_ASSIGN(BoxLayout)
public:
	BoxLayout(Orientation orientation);
	~BoxLayout();

	void add_widget(Widget *widget);

private:
#ifdef TINYUI_GTK
	Orientation m_orientation;
	bool m_expandable[2];
	GtkWidget *m_gtkwidget;
	bool expandable(Orientation orientation);
	GtkWidget *gtk_widget();
#endif
#ifdef TINYUI_QT
	QBoxLayout *m_qtlayout;
	QWidget *m_qtwidget;
	QWidget *qt_widget();
#endif
};

class Button;

class ButtonEvents {
public:
	virtual void clicked(Button *button) = 0;
};

class Button: public QObject, public Widget {
	DISABLE_ASSIGN(Button)
	Q_OBJECT

public:
	explicit Button(const std::wstring &label = std::wstring());
	~Button();

	void set_label(const std::wstring &label);
	void set_handler(ButtonEvents *handler);

private:
	ButtonEvents *m_handler;

#ifdef TINYUI_GTK
	GtkWidget *m_gtkwidget;
	bool expandable(Orientation orientation);
	GtkWidget *gtk_widget();
	static void clicked_cb(GtkWidget *widget, Button *button);
#endif
#ifdef TINYUI_QT
	QPushButton *m_qtwidget;
	QWidget *qt_widget();
private slots:
	void clicked_slot();
#endif
};

class ListBoxItem {
	DISABLE_ASSIGN(ListBoxItem)
	friend class ListBox;
public:
	ListBoxItem(const std::wstring &text = std::wstring());
	~ListBoxItem();

	void set_text(const std::wstring &text);

private:
#ifdef TINYUI_GTK
	std::wstring m_text;
	GtkTreeRowReference *m_rowref;
#endif
#ifdef TINYUI_QT
	QListWidgetItem *m_qtitem;
#endif
};

class ListBox;

class ListBoxEvents {
public:
	virtual void clicked(ListBox *listbox, ListBoxItem *item) = 0;
};

class ListBox: public QObject, public Widget {
	DISABLE_ASSIGN(ListBox)
	Q_OBJECT

public:
	ListBox();
	~ListBox();

	void set_handler(ListBoxEvents *handler);

	void add_item(ListBoxItem *item);
	void scroll_to(ListBoxItem *item);

private:
	ListBoxEvents *m_handler;

#ifdef TINYUI_GTK
	GtkWidget *m_gtkwidget, *m_treeview;
	GtkListStore *m_store;
	bool expandable(Orientation orientation);
	GtkWidget *gtk_widget();
	static void activated_cb(GtkTreeView *treeview, GtkTreePath *path,
				 GtkTreeViewColumn *col, ListBox *listbox);
#endif
#ifdef TINYUI_QT
	QListWidget *m_qtwidget;
	QWidget *qt_widget();
private slots:
	void itemActivated_slot(QListWidgetItem *qtitem);
#endif
};

class Window {
	DISABLE_ASSIGN(Window)
public:
	explicit Window(const std::wstring &title);
	~Window();
	void set_widget(Widget *widget);
	void show();

private:
#ifdef TINYUI_GTK
	GtkWidget *m_gtkwidget;
#endif
#ifdef TINYUI_QT
	QWidget *m_qtwidget;
	std::wstring m_title;
#endif
};

class Entry;

class EntryEvents {
public:
	virtual void activated(Entry *entry) = 0;
};

class Entry: public QObject, public Widget {
	DISABLE_ASSIGN(Entry)
	Q_OBJECT

public:
	explicit Entry(const std::wstring &text = std::wstring());
	~Entry();

	void set_text(const std::wstring &text);
	void set_handler(EntryEvents *handler);

	std::wstring get_text() const;

private:
	EntryEvents *m_handler;

#ifdef TINYUI_GTK
	GtkWidget *m_gtkwidget;
	bool expandable(Orientation orientation);
	GtkWidget *gtk_widget();
	static void activate_cb(GtkWidget *widget, Entry *Entry);
#endif
#ifdef TINYUI_QT
	QLineEdit *m_qtwidget;
	QWidget *qt_widget();
private slots:
	void returnPressed_slot();
#endif
};

class IoWatch;

class IoWatchEvents {
public:
	virtual void ready(IoWatch *iowatch, IoDirection dir) = 0;
};

class IoWatch: public QObject {
	DISABLE_ASSIGN(IoWatch)
	Q_OBJECT

public:
	explicit IoWatch(int fd, IoDirection dir);
	~IoWatch();

	void set_handler(IoWatchEvents *handler);

private:
	IoWatchEvents *m_handler;

#ifdef TINYUI_GTK
	GIOChannel *m_iochannel;
	int m_id;
	static bool io_watch_cb(GIOChannel *iochannel, GIOCondition cond,
				IoWatch *iowatch);
#endif
#ifdef TINYUI_QT
	QSocketNotifier *m_rd_notifier;
	QSocketNotifier *m_wr_notifier;
private slots:
	void rd_activated_slot();
	void wr_activated_slot();
#endif
};

class Timer;

class TimerEvents {
public:
	virtual void timeout(Timer *timer) = 0;
};

class Timer: public QObject {
public:
	explicit Timer(int interval);
	~Timer();

	void set_handler(TimerEvents *handler);

private:
	TimerEvents *m_handler;

#ifdef TINYUI_GTK
	guint m_id;
	static bool timer_cb(Timer *timer);
#endif
#ifdef TINYUI_QT
	int m_id;
	void timerEvent(QTimerEvent *event);
#endif
};

class QuitInterface
{
public:
	virtual void quit() = 0;
};

class SigIntHandler: private IoWatchEvents {
public:
	SigIntHandler();

	void set_handler(QuitInterface *handler);

private:
	QuitInterface *m_handler;
	int m_fd[2];

	void ready(IoWatch *iowatch, IoDirection dir);

	static SigIntHandler *m_instance;
	static void signal_handler(int sig);
};

class Application: public QuitInterface {
public:
	Application(int *argc, char ***argv);

	void quit();
	int run();

	static Application *instance();

private:
	static Application *m_instance;
};

}

#endif
