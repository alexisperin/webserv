/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi_login.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aperin <aperin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/06 14:25:29 by aperin            #+#    #+#             */
/*   Updated: 2023/06/15 14:46:36 by aperin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>

void send_error(std::string str)
{
	std::cout << "cgi_login: " << str << std::endl;
	exit(EXIT_FAILURE);
}

class Parse
{
	private:
		std::string _username;
		std::string _time;
		std::string _query;
		std::string _port;
		std::string _c_username;
		std::string _cookies;
		bool _form;
		bool _cookie;
		bool _logout;
	
	public:
		Parse(char **envp) : _form(false), _cookie(false), _logout(false)
		{
			for (size_t index = 0; envp[index]; index++)
			{
				std::string var = envp[index];
				if (!var.compare(0, 13, "QUERY_STRING="))
				{
					this->_query = var.substr(13);
					for (size_t sub_index = 0; this->_query[sub_index]; sub_index++)
					{
						if (this->_query[sub_index] == '+')
							this->_query[sub_index] = ' ';
					}
				}
				else if (!var.compare(0, 12, "SERVER_PORT="))
					this->_port = var.substr(12);
				else if (!var.compare(0, 12, "HTTP_COOKIE="))
					this->_cookies = var.substr(12);
			}
			if (this->_port.empty())
				send_error("SERVER_PORT missing");
		}
		~Parse(void) {}
		void set_info(void)
		{
			size_t userindex = this->_cookies.find("username=");
			if (userindex != std::string::npos)
			{
				size_t endindex = this->_cookies.find(';', userindex + 9);
				if (endindex != std::string::npos)
					this->_c_username = this->_cookies.substr(userindex + 9, endindex - (userindex + 9));
				else
					this->_c_username = this->_cookies.substr(userindex + 9);
			}
			if (this->_query.empty())
				return ;
			if (this->_query.compare(0, 9, "username="))
			{
				if (!this->_query.compare(0, 7, "logout="))
					this->_logout = true;
				else if (!this->_query.compare(0, 5, "form="))
					this->_form = true;
				else if (!this->_query.compare(0, 7, "cookie="))
					this->_cookie = true;
				else
					send_error("no username in query");
				return ;
			}
			size_t uindex = this->_query.find('&', 9);
			if (uindex == std::string::npos)
				send_error("no life in query");
			this->_username = this->_query.substr(9, uindex - 9);
			if (this->_query.compare(uindex + 1, 5, "life="))
				send_error("no life in query");
			size_t lindex = this->_query.find('&', uindex + 6);
			if (lindex == std::string::npos)
				send_error("no login in query");
			this->_time = this->_query.substr(uindex + 6, lindex - (uindex + 6));
		}
		void handle_request(void)
		{
			std::cerr << "port: " << this->_port << std::endl;
			if (this->_form)
			{
				std::cout << "HTTP/1.1 307 Temporary Redirect\n";
				std::cout << "location: http://localhost:";
				std::cout << this->_port;
				std::cout << "/../form.html\n\n";
			}
			else if (this->_cookie)
			{
				std::cout << "HTTP/1.1 307 Temporary Redirect\n";
				std::cout << "location: http://localhost:";
				std::cout << this->_port;
				std::cout << "/../cookie.html\n\n";
			}
			else if (this->_logout)
			{
				std::cout << "HTTP/1.1 307 Temporary Redirect\n";
				std::cout << "Set-Cookie: username=noting; Path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT\n";
				std::cout << "location: http://localhost:";
				std::cout << this->_port;
				std::cout << "/../login.html\n\n";
			}
			else if (!this->_username.empty())
			{
				std::cerr << "user: " << this->_username << "\ntime: " << this->_time << "\nquery: " << this->_query << std::endl;
				std::cout << "HTTP/1.1 307 Temporary Redirect\n";
				std::cout << "Set-Cookie: username=" << this->_username << "; ";
				if (!this->_time.empty())
					std::cout << "Max-Age=" << this->_time << "; ";
				std::cout << "Path=/\n";
				std::cout << "location: http://localhost:";
				std::cout << this->_port;
				std::cout << "/../cgi/login.cgi\n\n";
			}
			else if (!this->_c_username.empty())
			{
				std::cerr << "cookies: " << this->_cookies << "\nusername: " << this->_c_username << std::endl;
				std::cout << "HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: ";

				std::string html_content = "<html>\n<head>\n	<link rel=\"stylesheet\" href=\"../css/form.css\">\n";
				html_content += "</head> \n<body class=\"login\">\n<div class=\"global-box\">\n<h2>Successfully loged in as</h2>\n";
				html_content += "<h2>" + this->_c_username + "</h2>\n<form action=\"../cgi/login.cgi\">\n";
				html_content += "<button type=\"form\" name=\"form\">form</button>\n";
				html_content += "<button class=\"btn2\" type=\"cookie\" name=\"cookie\">cookie</button>\n";
				html_content += "<button class=\"btn3\" type=\"logout\" name=\"logout\">logout</button>\n</form>\n</div>\n</body>\n</html>";
				
				std::ostringstream content_length;
				content_length << html_content.size();
				std::cout << content_length.str() << "\n\n" << html_content << std::endl;
			}
			else
			{
				std::cout << "HTTP/1.1 307 Temporary Redirect\nlocation: http://localhost:";
				std::cout << this->_port;
				std::cout << "/../login.html\n\n";
			}
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