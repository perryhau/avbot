/**
 * @file
 * @author
 * @date
 * 
 * usage:
 * 	auto_question question;
 * 	auto_question::value_qq_list list;
 * 
 * 	list.push_back("lovey599");
 * 
 * 	questioin.add_to_list(list);
 * 	questioin.on_handle_message(group, qqclient);
 */

#ifndef __QQBOT_AUTO_QUESTION_H__
#define __QQBOT_AUTO_QUESTION_H__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include <boost/noncopyable.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "libwebqq/webqq.h"

static void qq_msg_send_callback(const boost::system::error_code& ec)
{
	// nothing to do
}

class auto_question : public boost::noncopyable
{
public:
	typedef std::vector<std::string> value_qq_list;
	
	auto_question(std::string filename = "question.txt")
		: _filename(filename)
		, _is_processing(false)
	{
		try
		{
			load_question();
		}
		catch (std::exception& ex)
		{
			std::cerr << "load questioin error." << std::endl;
		}
	}
	
	void add_to_list(value_qq_list list)
	{
		if (_is_processing)
		{
			_is_processing = false;
		}
		
		BOOST_FOREACH(std::string item, list)
		{
			_process_count.insert(std::pair<std::string, int>(item, _questioin_count));
		}
	}
	
	void on_handle_message(qqGroup& group, webqq& qqclient)
	{
		handle_question(group, qqclient);
	}
	
protected:
	void load_question()
	{
		std::fstream file(_filename.c_str(), std::ios_base::in);
		char question[512 + 1], answer[512 + 1];
		
		while (file.good())
		{
			file.getline(question, 512);
			file.getline(answer, 512);
			
			if (strlen(question) == 0)
			{
				continue;
			}
			
			_question.insert(std::pair<std::string, std::string>(question, answer));
		}
		
		file.close();
		
		_questioin_count = _question.size();
	}
	
	void handle_question(qqGroup& group, webqq& qqclient)
	{
		boost::asio::io_service io;
		std::map<std::string, int>::iterator iter;
		std::string str_msg_body = build_message();
		
		for (iter = _process_count.begin(); iter != _process_count.end(); ++iter)
		{
			std::string str_msg = boost::str(boost::format("@%1% 你好,欢迎你加入本群.请先回答以下问题:\n%2%谢谢\n") % iter->first % str_msg_body);
			
			std::cout << str_msg << std::endl;
			qqclient.send_group_message(group, str_msg, qq_msg_send_callback);
			
			io.reset();
			boost::asio::deadline_timer timer(io, boost::posix_time::seconds(1));
			timer.wait();
		}
	}
	
	std::string build_message()
	{
		value_question::iterator iter;
		std::string str_msg = "";
		int index = 1;
		
		for (iter = _question.begin(); iter != _question.end(); ++iter)
		{
			str_msg += boost::str(boost::format("问题%1%.%2%\n") % (index++) % iter->first);
		}
		
		return str_msg;
	}
	
private:
	std::string _filename;
	
	typedef std::map<std::string, std::string> value_question;
	value_question _question;
	
	int _questioin_count;
	
	bool _is_processing;
	
	std::map<std::string, int> _process_count;
};

#endif
