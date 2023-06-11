/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi_form.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/06 14:25:29 by aperin            #+#    #+#             */
/*   Updated: 2023/06/11 14:51:59 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>

void send_error(std::string str)
{
	std::cout << "cgi_form: " << str << std::endl;
	exit(EXIT_FAILURE);
}

std::string GET_content_type(std::string file)
{
	size_t size = file.size();
	if (size > 3)
	{
		if (!file.compare(size - 4, 4, ".css"))
			return ("text/css");
		else if (!file.compare(size - 4, 4, ".png"))
			return ("image/png");
		else if (!file.compare(size - 4, 4, ".ico"))
			return ("image/vnd.microsoft.icon");
		else if (!file.compare(size - 4, 4, ".pdf"))
			return ("application/pdf");
		else if (!file.compare(size - 4, 4, ".gif"))
			return ("image/gif");
		else if (!file.compare(size - 4, 4, ".jpg"))
			return ("image/jpg");
		else if (size > 4)
		{
			if (!file.compare(size - 5, 5, ".html"))
				return ("text/html");
			else if (!file.compare(size - 5, 5, ".jpeg"))
				return ("image/jpeg");
		}
	}
	return ("text/plain");
}

std::string read_data(std::ifstream &indata)
{
	std::string res;
	std::string line;
	while (!indata.eof()) {
		std::getline( indata, line );
		res += line;
		if (!indata.eof())
			res += '\n';
	}
	indata.close();
	return (res);
}

class Parse
{
	private:
		std::string _file;
		std::string _copy_file;
		std::string _root;
		std::string _query;
		std::string _method;
		std::string _port;
	
	public:
		Parse(char **envp)
		{
			for (size_t index = 0; envp[index]; index++)
			{
				std::string var = envp[index];
				if (!var.compare(0, 16, "PATH_TRANSLATED="))
					this->_root = var.substr(16);
				else if (!var.compare(0, 13, "QUERY_STRING="))
					this->_query = var.substr(13);
				else if (!var.compare(0, 12, "SERVER_PORT="))
					this->_port = var.substr(12);
			}
			if (this->_root.empty())
				send_error("PATH_TRANSLATED missing");
			if (this->_query.empty())
				send_error("QUERY_STRING missing");
			if (this->_port.empty())
				send_error("SERVER_PORT missing");
		}
		~Parse(void) {}
		void set_info(void)
		{
			if (this->_query.compare(0, 5, "file="))
				send_error("no file in query");
			size_t findex = this->_query.find('&', 5);
			if (findex == std::string::npos)
				send_error("no target in query");
			this->_file = this->_root + this->_query.substr(5, findex - 5);
			if (this->_query.compare(findex + 1, 7, "target="))
				send_error("no target in query");
			size_t tindex = this->_query.find('&', findex + 8);
			if (tindex == std::string::npos)
				send_error("no method in query");
			this->_copy_file = this->_root + this->_query.substr(findex + 8, tindex - (findex + 8));
			size_t mindex = this->_query.find('=', tindex + 1);
			if (mindex == std::string::npos)
				send_error("no method in query");
			this->_method = this->_query.substr(tindex + 1, mindex - (tindex + 1));
		}
		void handle_request(void)
		{
			std::cerr << "file: " << this->_file << "\ncopy file: " << this->_copy_file << "\nroot: " << this->_root << "\nquery: " << this->_query << "\nmethod: " << this->_method << "\nport: " << this->_port << std::endl;
			if (!this->_method.compare(0, 3, "get"))
			{
				std::cerr << "in get method" << std::endl;
				std::ifstream indata(this->_file.c_str());
				if (!indata.is_open())
				{
					std::cout << "HTTP/1.1 404 Not Found" << std::endl << std::endl;
					exit(EXIT_FAILURE);
				}

				std::string content = "HTTP/1.1 200 OK\nContent-Type: " + GET_content_type(this->_file) + "\nContent-Length: ";
				std::string file_content = read_data(indata);

				std::ostringstream content_length;
				content_length << file_content.size();
				content += content_length.str() + "\n\n" + file_content;
				std::cout << content;
			}
			else if (!this->_method.compare(0, 3, "put"))
			{
				std::string content;
				std::ifstream indata(this->_file.c_str());
				if (!indata.is_open())
				{
					std::cout << "HTTP/1.1 404 Not Found" << std::endl << std::endl;
					exit(EXIT_FAILURE);
				}
				std::string file_content = read_data(indata);
				indata.close();

				std::ofstream outdata(this->_copy_file.c_str(), std::ofstream::trunc);
				if (!outdata.is_open())
				{
					std::cout << "HTTP/1.1 404 Not Found" << std::endl << std::endl;
					exit(EXIT_FAILURE);
				}


				outdata << file_content;
				std::cout << "HTTP/1.1 307 Temporary Redirect\nlocation: http://localhost:";
				std::cout << this->_port;
				std::cout << "/form_put.html\n\n";
				
				outdata.close();
			}
			else if (!this->_method.compare(0, 3, "del"))
			{
				if (!std::remove(this->_file.c_str()))
				{
					std::cout << "HTTP/1.1 307 Temporary Redirect\nlocation: http://localhost:";
					std::cout << this->_port;
					std::cout << "/form_delete.html\n\n";
				}
				else
				{
					std::cout << "HTTP/1.1 404 Not Found\n\n";
				}
			}
			else
				std::cout << "HTTP/1.1 405 Method Not Allowed" << std::endl << std::endl;
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