#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "lirch/core/message.h"
#include "lirch/core/message_pipe.h"
#include "lirch/core/registry.h"

#include "lirch/plugins/parser.h"

using namespace std;

class test_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new test_message);}
	static message create(const std::string &s) {return message_create(s, new test_message);}
};

int dotest(string test)
{
	bidirectional_message_pipe mp;
	if (test=="toplugin")
	{
		mp.core_write(test_message::create("Hello, plugin"));
		if (mp.plugin_read().type!="Hello, plugin")
			return 1;
		return 0;
	}
	if (test=="tocore")
	{
		mp.plugin_write(test_message::create("Hello, core"));
		if (mp.core_read().type!="Hello, core")
			return 1;
		return 0;
	}
	if (test=="readempty")
	{
		if (mp.plugin_read().type!="")
			return 1;
		if (mp.core_read().type!="")
			return 1;
		return 0;
	}
	if (test=="cycle")
	{
		mp.plugin_write(test_message::create("Whee!"));
		mp.core_write(mp.core_read());
		mp.plugin_write(mp.plugin_read());
		if (mp.core_read().type!="Whee!")
			return 1;
		return 0;
	}
	if (test=="exhaust")
	{
		mp.plugin_write(test_message::create("Yukkuri shite itte ne"));
		if (mp.core_read().type!="Yukkuri shite itte ne")
			return 1;
		if (mp.core_read().type!="")
			return 1;
		if (mp.plugin_read().type!="")
			return 1;
		return 0;
	}
	if (test=="sharing")
	{
		message_pipe shared;
		bidirectional_message_pipe mp1(message_pipe(), shared);
		bidirectional_message_pipe mp2(message_pipe(), shared);
		mp1.plugin_write(test_message::create("I said hey!"));
		if (mp1.core_peek().type!="I said hey!")
			return 1;
		if (mp2.core_read().type!="I said hey!")
			return 1;
		if (mp1.core_read().type!="")
			return 1;
		mp1.core_write(test_message::create("Whats going on?"));
		if (mp2.plugin_read().type!="")
			return 1;
		if (mp1.plugin_read().type!="Whats going on?")
			return 1;
		return 0;
	}
	if (test=="order")
	{
		mp.plugin_write(test_message::create("Message 1"));
		mp.core_write(test_message::create("Message 2"));
		mp.plugin_write(test_message::create("Message 3"));
		if (mp.core_read().type!="Message 1")
			return 1;
		if (mp.plugin_read().type!="Message 2")
			return 1;
		if (mp.core_read().type!="Message 3")
			return 1;
		return 0;
	}
	if (test=="copying")
	{
		mp.plugin_write(test_message::create("open the pod bay doors hal"));
		bidirectional_message_pipe mp2=mp;
		if (mp2.core_read().type!="open the pod bay doors hal")
			return 1;
		cout << "part1\n";
		mp2.core_write(test_message::create("I'm sorry Dave"));
		mp.core_write(test_message::create("I'm afraid I can't do that"));
		if (mp2.plugin_read().type!="I'm sorry Dave")
			return 1;
		bidirectional_message_pipe mp3=mp2;
		if (mp3.plugin_read().type!="I'm afraid I can't do that")
			return 1;
		mp2.plugin_write(test_message::create("whats the problem"));
		if (mp3.core_read().type!="whats the problem")
			return 1;
		mp3.core_write(test_message::create("Your grammar is too terrible, Dave"));
		if (mp.plugin_read().type!="Your grammar is too terrible, Dave")
			return 1;
		return 0;
	}
	if (test=="threadseq")
	{
		thread t1([&mp]()
		{
			mp.plugin_write(test_message::create("na"));
		});
		thread t2([&mp]()
		{
			mp.plugin_write(test_message::create("na"));
		});
		t1.join();
		t2.join();
		if (mp.core_read().type!="na")
			return 1;
		if (mp.core_read().type!="na")
			return 1;
		if (mp.core_read().type!="")
			return 1;
		return 0;
	}
	if (test=="0bread")
	{
		mp.core_write(test_message::create("wa pan nashi!"));
		if (mp.plugin_blocking_read().type!="wa pan nashi!")
			return 1;
		return 0;
	}
	if (test=="1bread")
	{
		thread t([&mp]()
		{
			this_thread::sleep_for(chrono::milliseconds(100));
			mp.plugin_write(test_message::create("Threading"));
		});
		if (mp.core_blocking_read().type!="Threading")
		{
			t.join();
			return 1;
		}
		t.join();
		return 0;
	}
	if (test=="2bread")
	{
		thread t([&mp]()
		{
			string s=mp.plugin_blocking_read().type;
			this_thread::sleep_for(chrono::milliseconds(100));
			if (s!="Batman")
				mp.plugin_write(test_message::create("Aquaman"));
			else
				mp.plugin_write(test_message::create("Superman"));
		});
		mp.plugin_write(test_message::create("Spiderman"));
		mp.core_write(test_message::create("Batman"));
		if (mp.core_blocking_read().type!="Spiderman")
		{
			t.join();
			return 1;
		}
		if (mp.core_blocking_read().type!="Superman")
		{
			t.join();
			return 1;
		}
		t.join();
		return 0;
	}
	if (test=="shared2bread")
	{
		thread t1([&mp]()
		{
			if (mp.plugin_blocking_read().type!="rm")
				mp.plugin_write(test_message::create("/"));
			else
				mp.plugin_write(test_message::create("-rf"));
		});
		thread t2([&mp]()
		{
			if (mp.plugin_blocking_read().type!="rm")
				mp.plugin_write(test_message::create("/"));
			else
				mp.plugin_write(test_message::create("-rf"));
		});
		this_thread::sleep_for(chrono::milliseconds(100));
		mp.core_write(test_message::create("rm"));
		if (mp.core_blocking_read().type!="-rf")
		{
			mp.core_write(test_message::create(""));
			mp.core_write(test_message::create(""));
			t1.join();
			t2.join();
			return 1;
		}
		if (mp.core_read().type!="")
		{
			mp.core_write(test_message::create(""));
			mp.core_write(test_message::create(""));
			t1.join();
			t2.join();
			return 1;
		}
		mp.core_write(test_message::create("rm"));
		if (mp.core_blocking_read().type!="-rf")
		{
			mp.core_write(test_message::create(""));
			mp.core_write(test_message::create(""));
			t1.join();
			t2.join();
			return 1;
		}
		t1.join();
		t2.join();
		return 0;
	}
	if (test=="register")
	{
		registry r;
		r.add(0,"later");
		r.add(100,"earlier");
		if (r.get(32767).second!="earlier")
			return 1;
		if (r.get(100).second!="earlier")
			return 1;
		if (r.get(50).second!="later")
			return 1;
		if (r.get(0).second!="later")
			return 1;
		if (r.get(-12).second!="")
			return 1;
		return 0;
	}
	if (test=="registryInitList")
	{
		registry r{{100,"earlier"},{0,"later"}};
		if (r.get(32767).second!="earlier")
			return 1;
		if (r.get(100).second!="earlier")
			return 1;
		if (r.get(50).second!="later")
			return 1;
		if (r.get(0).second!="later")
			return 1;
		if (r.get(-12).second!="")
			return 1;
		return 0;
	}
	if (test=="registryEmpty")
	{
		registry r;
		if (!r.empty())
			return 1;
		r.add(0,"name");
		if (r.empty())
			return 1;
		if (r.size()!=1)
			return 1;
		r.clear();
		if (!r.empty())
			return 1;
		if (r.size()!=0)
			return 1;
		return 0;
	}
	if (test=="registryDups")
	{
		registry r;
		if (!r.add(0,"foo"))
			return 1;
		if (r.add(0,"foo"))
			return 1;
		if (r.add(0,"bar"))
			return 1;
		return 0;
	}
	if (test=="parse1")
	{
		auto i=parse("");
		if (i.size()!=0)
			return 1;
		return 0;
	}
	if (test=="parse2")
	{
		auto i=parse(" ");
		if (i.size()!=1)
			return 1;
		if (i[0]!="")
			return 1;
		return 0;
	}
	if (test=="parse3")
	{
		auto i=parse(R"*(hello, is it me you\'re looking for?)*");
		if (i.size()!=7)
			return 1;
		if (i[0]!="hello,")
			return 1;
		if (i[1]!="is")
			return 1;
		if (i[2]!="it")
			return 1;
		if (i[3]!="me")
			return 1;
		if (i[4]!="you\'re")
			return 1;
		if (i[5]!="looking")
			return 1;
		if (i[6]!="for?")
			return 1;
		return 0;
	}
	if (test=="parse4")
	{
		auto i=parse(R"*(/channel 'l33t h4xx0r5')*");
		if (i.size()!=2)
			return 1;
		if (i[0]!="/channel")
			return 1;
		if (i[1]!="l33t h4xx0r5")
			return 1;
		return 0;
	}
	if (test=="parse5")
	{
		auto i=parse(R"*(\U01F438)*");
		if (i.size()!=1)
			return 1;
		uint frog_face=0x1f438;
		if (i[0]!=QString::fromUcs4(&frog_face, 1))
			return 1;
		return 0;
	}
	if (test=="parse6")
	{
		auto i=parse(R"*(daijoubu\n yuki?)*");
		if (i.size()!=2)
			return 0;
		if (i[0]!="daijoubu\n")
			return 1;
		if (i[1]!="yuki?")
			return 1;
		return 0;
	}
	if (test=="parse7")
	{
		auto i=parse(R"*(My     name     ...            is       William                 Shatner.   )*");
		if (i.size()!=6)
			return 0;
		if (i[0]!="My")
			return 1;
		if (i[1]!="name")
			return 1;
		if (i[2]!="...")
			return 1;
		if (i[3]!="is")
			return 1;
		if (i[4]!="William")
			return 1;
		if (i[5]!="Shatner.")
			return 1;
		return 0;
	}
	if (test=="parse8")
	{
		auto i=parse(R"*(I want"to break"free)*");
		if (i.size()!=2)
			return 1;
		if (i[0]!="I")
			return 1;
		if (i[1]!="wantto breakfree")
			return 1;
		return 0;
	}
	if (test=="parse9")
	{
		auto i=parse(R"*(from you're lies \you're so self-satis"fied I don\'t ne"ed you)*");
		if (i.size()!=5)
			return 1;
		if (i[0]!="from")
			return 1;
		if (i[1]!="youre lies \\youre")
			return 1;
		if (i[2]!="so")
			return 1;
		if (i[3]!="self-satisfied I don't need")
			return 1;
		if (i[4]!="you")
			return 1;
		return 0;
	}
	return 2;
}

int main(int argc, char *argv[])
{
	if (argc<2)
		return 0;
	bool verbose=false;
	int n=1;
	while (argv[n]!=NULL && argv[n][0]=='-')
	{
		for (int o=1;argv[n][o]!='\0';++o)
		{
			if (argv[n][o]=='v')
				verbose=true;
			else if (argv[n][o]=='q')
				verbose=false;
			else
			{
				cout << "Unsupported option '" << argv[n][o] << "'\n";
				return 3;
			}
		}
		++n;
	}
	if (argv[n]==NULL)
	{
		cout << "No test specified\n";
		return 3;
	}
	int ret=dotest(argv[n]);
	if (verbose)
	{
		if (ret==0)
			cout << "Test " << argv[n] << " was successful\n";
		else if (ret==1)
			cout << "Test " << argv[n] << " failed\n";
		else if (ret==2)
			cout << "Unsupported test " << argv[n] << endl;
	}
	return ret;
}
