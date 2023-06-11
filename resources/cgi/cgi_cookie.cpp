/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi_cookie.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/06 14:25:29 by aperin            #+#    #+#             */
/*   Updated: 2023/06/11 17:59:56 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>

void send_error(std::string str)
{
	std::cout << "cgi_cookie: " << str << std::endl;
	exit(EXIT_FAILURE);
}

class Parse
{
	private:
		std::string _key;
		std::string _value;
		std::string _time;
		std::string _query;
		std::string _button;
		std::string _port;
	
	public:
		Parse(char **envp)
		{
			for (size_t index = 0; envp[index]; index++)
			{
				std::string var = envp[index];
				if (!var.compare(0, 13, "QUERY_STRING="))
					this->_query = var.substr(13);
				else if (!var.compare(0, 12, "SERVER_PORT="))
					this->_port = var.substr(12);
			}
			if (this->_query.empty())
				send_error("QUERY_STRING missing");
			if (this->_port.empty())
				send_error("SERVER_PORT missing");
		}
		~Parse(void) {}
		void set_info(void)
		{
			if (this->_query.compare(0, 4, "key="))
				send_error("no key in query");
			size_t kindex = this->_query.find('&', 4);
			if (kindex == std::string::npos)
				send_error("no value in query");
			this->_key = this->_query.substr(4, kindex - 4);
			if (this->_query.compare(kindex + 1, 6, "value="))
				send_error("no value in query");
			size_t vindex = this->_query.find('&', kindex + 7);
			if (vindex == std::string::npos)
				send_error("no life in query");
			this->_value = this->_query.substr(kindex + 7, vindex - (kindex + 7));
			size_t lindex = this->_query.find('=', vindex + 1);
			if (lindex == std::string::npos)
				send_error("no life in query");
			size_t bindex = this->_query.find('&', lindex + 1);
			if (bindex == std::string::npos)
				send_error("no button in query");
			this->_time = this->_query.substr(lindex + 1, bindex - (lindex + 1));
			this->_button = this->_query.substr(bindex + 1, this->_query.size() - (bindex + 2));
		}
		void handle_request(void)
		{
			std::cerr << "key: " << this->_key << "\nvalue: " << this->_value << "\ntime: " << this->_time << "\nbutton: " << this->_button << "\nquery: " << this->_query << "\nport: " << this->_port << std::endl;
			if (!this->_button.compare(0, 3, "set"))
			{
				std::cout << "HTTP/1.1 205 Reset Content\n";
				std::cout << "Set-Cookie: " << this->_key << '=' << this->_value << "; ";
				if (!this->_time.empty())
					std::cout << "Max-Age=" << this->_time << "; ";
				std::cout << "Path=/\n\n";
			}
			else if (!this->_button.compare(0, 6, "remove"))
			{
				std::cout << "HTTP/1.1 205 Reset Content\n";
				std::cout << "Set-Cookie: " << this->_key << '=' << this->_value << "; ";
				std::cout << "Path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT\n\n";
			}
			else if (!this->_button.compare(0, 8, "printenv"))
			{
				std::cout << "HTTP/1.1 307 Temporary Redirect\nlocation: http://localhost:";
				std::cout << this->_port;
				std::cout << "/cgi/printenv.pl\n\n";
			}
			else
				std::cout << "HTTP/1.1 500  Internal Server Error" << std::endl << std::endl;
		}
};

int main(int ac, char **av, char **envp)
{
	(void)av;
	if (ac != 1)
		send_error("takes no argument");
	
	Parse parsing(envp);

	parsing.set_info();
	parsing.handle_request();
	return (0);
}