// This is sometimes required to support the full curses API
#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#ifdef __GNUC__
	// Enable printf and scanf argument checking
	#define GCC_PRINTF
	#define	GCC_SCANF
#endif

#include <climits>
#include <cstdio>
#include <cwctype>
#include <unordered_map>

#include <curses.h>

#include <QString>
#include <QTextBoundaryFinder>

#include "lirch_constants.h"

#include "lirch/core/core_messages.h"

#include "lirch/plugins/lirch_plugin.h"
#include "lirch/plugins/hash/QString.h"

#include "lirch/plugins/messages/channel_messages.h"
#include "lirch/plugins/messages/display_messages.h"
#include "lirch/plugins/messages/edict_messages.h"
#include "lirch/plugins/messages/notify_messages.h"

#include "lirch/ui/line_editor.h"

inline char CTRL(char c)
{
	//Leave only the lower 5 bits, so the return value is the numeric value
	//of CTRL+key (e.g. CTRL('c') gives you 3, which is the value you get
	//when hitting ctrl-c)
	return c&0x1f;
}

using namespace std;

template <class... args>
string strprintf(const string &format, args... a)
{
	//Thankfully, the C++ standard grabbed the definition from POSIX
	//instead of Windows, so I don't have to binary-search the correct
	//string size.
	int size=snprintf(NULL, 0, format.c_str(), a...);
	//Add padding for the terminating byte
	vector<char> s(size+1);
	//We can use sprintf instead of snprintf because we know the buffer is large enough
	sprintf(s.data(), format.c_str(), a...);
	return s.data();
}

//Same as wprintf, but handles unicode characters properly
template <class... args>
void wprintu(WINDOW *w, const string &format, args... a)
{
	string s=strprintf(format, a...);
	wstring wstr=QString::fromLocal8Bit(s.c_str()).toStdWString();
	waddwstr(w, wstr.c_str());
}

//This wraps WINDOW pointers so they're be destroyed on exit
class WindowWrapper
{
public:
	WindowWrapper(WINDOW *ww) : w(ww, delwin) {}
	WindowWrapper() {int maxx, maxy; getmaxyx(stdscr, maxy, maxx); w=shared_ptr<WINDOW>(newpad(maxy-1, maxx), delwin); scrollok(get(), TRUE);}
	WINDOW *get() const {return w.get();}
	operator WINDOW*() const {return get();}
	WINDOW& operator*() const {return *w;}
	WINDOW* operator->() const {return get();}
private:
	shared_ptr<WINDOW> w;
};

void runplugin(plugin_pipe &p, const string &name)
{
	text_line input;
	QString channel="default";
	int maxx, maxy;
	getmaxyx(stdscr, maxy, maxx);
	//10000 lines of scrollback should be enough for anyone
	//WindowWrapper channel_output=newpad(10000, maxx);
	unordered_map<QString, WindowWrapper> channel_windows;
	if (channel_windows["default"]==nullptr)
		return;
	WindowWrapper input_display=newwin(1, maxx-1, maxy-1, 0);
	if (input_display==nullptr)
		return;
	//Be lazy and let the input scroll
	scrollok(input_display, TRUE);
	//p.write(registration_message::create(-30000, name, "display"));
	while (true)
	{
		wint_t key;
		int rc=get_wch(&key);
		if (rc==OK)
		{
			if (key=='\r' || key=='\n')
			{
				p.write(raw_edict_message::create(input.getQString(),channel));
				input.kill_whole_line();
			}
			else if (key==(unsigned char)CTRL('U'))
			{
				input.backward_kill_line();
			}
			else if (key==(unsigned char)CTRL('H'))
			{
				input.backward_delete_char();
			}
			else if (key==WEOF)
			{
				//Quit since we have no more input
				break;
			}
			else
			{
				//This conversion is always valid, since key's
				//only valid values are valid wchar_t values
				//and WEOF, and we know it's not WEOF.
				input.insert(key);
			}
		}
		else if (rc==KEY_CODE_YES)
		{
			if (key==KEY_BACKSPACE)
			{
				input.backward_delete_char();
			}
			else if (key==KEY_ENTER)
			{
				p.write(raw_edict_message::create(input.getQString(),channel));
				input.kill_whole_line();
			}
		}
		while (p.has_message())
		{
			message m=p.read();
			if (m.type=="shutdown")
			{
				return;
			}
			else if (m.type=="registration_status")
			{
				auto s=dynamic_cast<registration_status *>(m.getdata());
				if (!s)
					continue;
				if (!s->status)
				{
					if (s->priority>-32000)
						p.write(registration_message::create(s->priority-1, name, s->type));
				}
			}
			else if (m.type=="display")
			{
				auto s=dynamic_cast<display_message *>(m.getdata());
				if (!s)
					continue;
				p.write(m.decrement_priority());

				QString message_channel=s->channel;
				string nick=s->nick.toLocal8Bit().constData();
				string contents=s->contents.toLocal8Bit().constData();

				if (s->channel=="")
					message_channel=channel;

				if (channel_windows.count(message_channel)!=0)
				{
					if(s->subtype==display_message_subtype::NORMAL)
						wprintu(channel_windows[message_channel], "<%s> %s\n", nick.c_str(), contents.c_str());
					if(s->subtype==display_message_subtype::ME)
						wprintu(channel_windows[message_channel], "* %s %s\n", nick.c_str(), contents.c_str());
					if(s->subtype==display_message_subtype::NOTIFY)
						wprintu(channel_windows[message_channel], QString((QChar[]){0x203C, 0x203D, ' ', '%', 's', '\n', 0}).toLocal8Bit().constData(), contents.c_str());
					if(s->subtype==display_message_subtype::NOTIFY_CURRENT)
						wprintu(channel_windows[message_channel], QString((QChar[]){0x203C, 0x203C, 0x203D, ' ', '%', 's', '\n', 0}).toLocal8Bit().constData(), contents.c_str());
				}
			}
			else if (m.type=="set_channel")
			{
				auto i=dynamic_cast<set_channel_message *>(m.getdata());
				if (!i)
					continue;
				p.write(m.decrement_priority());
				if (i->channel=="")
					p.write(notify_message::create(channel, "On channel "+channel));
				else
					channel=i->channel;
			}
			else if (m.type=="leave_channel")
			{
				auto i=dynamic_cast<leave_channel_message *>(m.getdata());
				if (!i)
					continue;
				p.write(m.decrement_priority());
				if (i->channel=="")
					i->channel=channel;
				channel_windows.erase(i->channel);
				if (i->channel==channel)
					p.write(set_channel_message::create(channel));
			}
			else
				p.write(m.decrement_priority());
		}
		int x,y;
		getyx(channel_windows[channel], y, x);
		pnoutrefresh(channel_windows[channel], max(y-(maxy-1),0), 0, 0,0,maxy-2,maxx-1);
		wmove(input_display, 0, 0);
		wprintu(input_display, "\n%s", input.getQString().toLocal8Bit().constData());
		wnoutrefresh(input_display);
		//This doesn't need to run all the time, but we should be able
		//to cope with the screen refreshing at 10Hz
		doupdate();
	}
	p.write(core_quit_message::create());
}

void run(plugin_pipe p, string name)
{
	p.write(registration_message::create(-30000, name, "display"));
	p.write(registration_message::create(-30000, name, "set_channel"));
	p.write(registration_message::create(-30000, name, "leave_channel"));
	//Set the delay when hitting escape to 10 milliseconds, unless it was
	//already set.  The ESCDELAY variable is not supported in all curses
	//implementations, but should not cause problems in implementations
	//that ignore it.
	setenv("ESCDELAY", "10", 0);
	//Initialize curses
	initscr();
	//Don't buffer typed characters
	cbreak();
	//Wain no more than a tenth of a second for input
	timeout(100);
	noecho();
	//Makes enter return \r instead of \n
	nonl();
	//Flush the input buffer if an interrupt key is pressed.  This ensures
	//we don't miss keystrokes in the event of a SIGSTOP
	intrflush(stdscr, FALSE);
	//Enable keycodes
	keypad(stdscr, TRUE);
	runplugin(p, name);
	//Make sure to always restore the terminal to a sane configuration
	endwin();
	return;
}
