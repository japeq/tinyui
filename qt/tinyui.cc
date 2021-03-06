/*
 * Qt 4 implementation of tinyui GUI classes
 */
#include "tiny_ui.h"
#include <stdexcept>
#include <QBoxLayout>
#include <QPushButton>
#include <QSocketNotifier>
#include <QListWidget>
#include <QLineEdit>
#include <QApplication>
#include <QTimerEvent>

namespace tinyui {

void Widget::show()
{
	qt_widget()->show();
}

void Widget::hide()
{
	qt_widget()->hide();
}

BoxLayout::BoxLayout(Orientation orientation)
{
	switch (orientation) {
	case HORIZONTAL:
		m_qtlayout = new QHBoxLayout;
		break;
	case VERTICAL:
		m_qtlayout = new QVBoxLayout;
		break;
	default:
		throw std::runtime_error("Invalid orientation");
	}
	m_qtwidget = new QWidget;
	m_qtwidget->setLayout(m_qtlayout);
}

BoxLayout::~BoxLayout()
{
	delete m_qtwidget;
}

void BoxLayout::add_widget(Widget *widget)
{
	m_qtlayout->addWidget(widget->qt_widget());
}

QWidget *BoxLayout::qt_widget()
{
	return m_qtwidget;
}

Button::Button(const std::wstring &label) :
	m_handler(NULL)
{
	m_qtwidget = new QPushButton(QString::fromStdWString(label));
	connect(m_qtwidget, SIGNAL(clicked()), SLOT(clicked_slot()));
}

Button::~Button()
{
	delete m_qtwidget;
}

void Button::set_label(const std::wstring &label)
{
	m_qtwidget->setText(QString::fromStdWString(label));
}

QWidget *Button::qt_widget()
{
	return m_qtwidget;
}

void Button::clicked_slot()
{
	if (m_handler)
		m_handler->clicked(this);
}

ListBoxItem::ListBoxItem(const std::wstring &text)
{
	m_qtitem = new QListWidgetItem(QString::fromStdWString(text));
	m_qtitem->setData(Qt::UserRole, QVariant::fromValue<void *>(this));
}

ListBoxItem::~ListBoxItem()
{
	delete m_qtitem;
}

void ListBoxItem::set_text(const std::wstring &text)
{
	m_qtitem->setText(QString::fromStdWString(text));
}

ListBox::ListBox() :
	m_handler(NULL)
{
	m_qtwidget = new QListWidget;
	connect(m_qtwidget, SIGNAL(itemActivated(QListWidgetItem *)),
		SLOT(itemActivated_slot(QListWidgetItem *)));
}

ListBox::~ListBox()
{
	delete m_qtwidget;
}

void ListBox::add_item(ListBoxItem *item)
{
	m_qtwidget->addItem(item->m_qtitem);
}

void ListBox::scroll_to(ListBoxItem *item)
{
	m_qtwidget->scrollToItem(item->m_qtitem);
}

QWidget *ListBox::qt_widget()
{
	return m_qtwidget;
}

void ListBox::itemActivated_slot(QListWidgetItem *qtitem)
{
	ListBoxItem *item =
		reinterpret_cast<ListBoxItem *>(qtitem->data(Qt::UserRole).value<void *>());
	if (m_handler)
		m_handler->clicked(this, item);
}

Window::Window(const std::wstring &title) :
	m_title(title)
{
}

Window::~Window()
{
}

void Window::set_widget(Widget *widget)
{
	m_qtwidget = widget->qt_widget();
	m_qtwidget->setWindowTitle(QString::fromStdWString(m_title));
}

void Window::show()
{
	m_qtwidget->show();
}

Entry::Entry(const std::wstring &text) :
	m_handler(NULL)
{
	m_qtwidget = new QLineEdit(QString::fromStdWString(text));
	connect(m_qtwidget, SIGNAL(returnPressed()), SLOT(returnPressed_slot()));
}

Entry::~Entry()
{
	delete m_qtwidget;
}

void Entry::set_text(const std::wstring &text)
{
	m_qtwidget->setText(QString::fromStdWString(text));
}

std::wstring Entry::get_text() const
{
	return m_qtwidget->text().toStdWString();
}

QWidget *Entry::qt_widget()
{
	return m_qtwidget;
}

void Entry::returnPressed_slot()
{
	if (m_handler)
		m_handler->activated(this);
}

IoWatch::IoWatch(int fd, IoDirection dir) :
	m_handler(NULL), m_rd_notifier(NULL), m_wr_notifier(NULL)
{
	if (dir & IN) {
		m_rd_notifier = new QSocketNotifier(fd, QSocketNotifier::Read);
		connect(m_rd_notifier, SIGNAL(activated(int)),
			SLOT(rd_activated_slot()));
	}
	if (dir & OUT) {
		m_wr_notifier = new QSocketNotifier(fd, QSocketNotifier::Write);
		connect(m_wr_notifier, SIGNAL(activated(int)),
			SLOT(wr_activated_slot()));
	}
}

IoWatch::~IoWatch()
{
	delete m_rd_notifier;
	delete m_wr_notifier;
}

void IoWatch::rd_activated_slot()
{
	if (m_handler)
		m_handler->ready(this, IN);
}

void IoWatch::wr_activated_slot()
{
	if (m_handler)
		m_handler->ready(this, OUT);
}

Timer::Timer(int interval) :
	m_handler(NULL)
{
	m_id = startTimer(interval);
}

Timer::~Timer()
{
}

void Timer::timerEvent(QTimerEvent *event)
{
	if (event->timerId() != m_id)
		return;
	if (m_handler)
		m_handler->timeout(this);
}

Application::Application(int *argc, char ***argv)
{
	if (m_instance)
		throw std::runtime_error("Application instance already created");
	m_instance = this;
	new QApplication(*argc, *argv);
}

void Application::quit()
{
	qApp->quit();
}

int Application::run()
{
	return qApp->exec();
}

}
