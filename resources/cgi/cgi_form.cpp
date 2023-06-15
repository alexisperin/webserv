/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi_form.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aperin <aperin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/06 14:25:29 by aperin            #+#    #+#             */
/*   Updated: 2023/06/15 14:52:48 by aperin           ###   ########.fr       */
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
		std::string _request;
		std::string _content_type;
	
	public:
		Parse(char **envp)
		{
			for (size_t index = 0; envp[index]; index++)
			{
				std::string var = envp[index];
				if (!var.compare(0, 16, "PATH_TRANSLATED="))
					this->_root = var.substr(16);
				else if (!var.compare(0, 13, "QUERY_STRING="))
				{
					this->_query = var.substr(13);
					for (size_t index = 0; this->_query[index]; index++)
					{
						if (this->_query[index] == '+')
							this->_query[index] = ' ';
					}
				}
				else if (!var.compare(0, 12, "SERVER_PORT="))
					this->_port = var.substr(12);
				else if (!var.compare(0, 15, "REQUEST_METHOD="))
					this->_request = var.substr(15);
				else if (!var.compare(0, 13, "CONTENT_TYPE="))
					this->_content_type = var.substr(13);
			}
			if (this->_root.empty())
				send_error("PATH_TRANSLATED missing");
			if (this->_port.empty())
				send_error("SERVER_PORT missing");
		}
		~Parse(void) {}
		void set_info(void)
		{
			if (this->_request.compare(0, 3, "GET"))
				return ;
			if (this->_query.compare(0, 5, "file="))
				send_error("no file in query");
			size_t findex = this->_query.find('&', 5);
			if (findex == std::string::npos)
				send_error("no target in query");
			this->_file = this->_root + "uploads/" + this->_query.substr(5, findex - 5);
			size_t tindex;
			if (!this->_query.compare(findex + 1, 7, "target="))
			{
				tindex = this->_query.find('&', findex + 8);
				if (tindex == std::string::npos)
					send_error("no method in query");
				this->_copy_file = this->_root + this->_query.substr(findex + 8, tindex - (findex + 8));
			}
			else
				tindex = findex;
			size_t mindex = this->_query.find('=', tindex + 1);
			if (mindex == std::string::npos)
				send_error("no method in query");
			this->_method = this->_query.substr(tindex + 1, mindex - (tindex + 1));
		}
		void handle_request(void)
		{
			std::cerr << "file: " << this->_file << "\ncopy file: " << this->_copy_file << "\nroot: " << this->_root << "\nquery: " << this->_query << "\nmethod: " << this->_method << "\nport: " << this->_port << std::endl;
			std::cerr << "request: " << this->_request << std::endl;
			if (!this->_method.compare(0, 3, "get"))
			{
				std::cerr << "in get method" << std::endl;
				std::ifstream indata(this->_file.c_str());
				if (!indata.is_open())
				{
					std::cout << "HTTP/1.1 307 Temporary Redirect\nlocation: http://localhost:";
					std::cout << this->_port;
					std::cout << "/forms/form_get_failure.html\n\n";
					return ;
				}

				std::string content = "HTTP/1.1 200 OK\nContent-Type: " + GET_content_type(this->_file) + "\nContent-Length: ";
				std::string file_content = read_data(indata);

				std::ostringstream content_length;
				content_length << file_content.size();
				content += content_length.str() + "\r\n\r\n" + file_content;
				std::cout << content;
			}
			else if (!this->_request.compare(0, 4, "POST"))
			{
				std::cerr << "content-type: " << this->_content_type << std::endl;
				std::string boundary;
				if (!this->_content_type.compare(0, 20, "multipart/form-data;"))
				{
					size_t boundindex = this->_content_type.find("boundary=");
					if (boundindex != std::string::npos)
						boundary = this->_content_type.substr(boundindex + 9, this->_content_type.find("\r\n", boundindex + 9) - (boundindex + 7));
					else
						send_error("no boundary in post");
				}

				std::cerr << "boundary: " << boundary << std::endl;

				std::string body;
				std::string line;
				while (!std::cin.eof()) {
					std::getline( std::cin, line );
					// std::cerr << "new line of size " << line.size() << std::endl;
					body.append(line);
					if (!std::cin.eof())
						body += '\n';
				}

				std::cerr << "body size: " << body.size() << std::endl;

				size_t first_bound = body.find(boundary);
				if (first_bound == std::string::npos)
					send_error("no first boundary in body");
				size_t first_body = body.find("\r\n\r\n", first_bound);
				if (first_body == std::string::npos)
					send_error("no first body in body");
				size_t second_bound = body.find(boundary, first_body);
				if (second_bound == std::string::npos)
					send_error("no second boundary in body");
				second_bound -= 4;
				std::string file_content = body.substr(first_body + 4, second_bound - (first_body + 4));
				size_t second_body = body.find("\r\n\r\n", second_bound);
				if (second_body == std::string::npos)
					send_error("no second body in body");
				size_t third_bound = body.find(boundary, second_body);
				if (third_bound == std::string::npos)
					send_error("no third boundary in body");
				third_bound -= 4;
				this->_copy_file = "resources/uploads/" + body.substr(second_body + 4, third_bound - (second_body + 4));


				// std::cerr << body << std::endl;
				std::ofstream outdata(this->_copy_file.c_str(), std::ofstream::trunc);
				if (!outdata.is_open())
				{
					std::cout << "HTTP/1.1 404 Not Found" << std::endl << std::endl;
					exit(EXIT_FAILURE);
				}
				outdata << file_content;
				outdata.close();

				std::cout << "HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: ";

				std::string html_content = "<html>\n<head>\n\t<link rel=\"stylesheet\" href=\"../css/form.css\">\n</head> \n<body>\n\t<div class=\"global-box\">\n\t\t<h2>Upload</h2>\n\t\t<h2>Your file has succesfully been uploaded!</h2>\n\t\t<form action=\"../form.html\">\n\t\t\t<button class=\"btn3\" type=\"back\">back</button>\n\t\t</form>\n\t</div>\n</body>\n</html>";
				
				std::ostringstream content_length;
				content_length << html_content.size();
				std::cout << content_length.str() << "\r\n\r\n" << html_content << std::endl;
			
				
			}
			else if (!this->_method.compare(0, 3, "del"))
			{
				if (!std::remove(this->_file.c_str()))
				{
					std::cout << "HTTP/1.1 307 Temporary Redirect\nlocation: http://localhost:";
					std::cout << this->_port;
					std::cout << "/forms/form_delete_success.html\n\n";
				}
				else
				{
					std::cout << "HTTP/1.1 307 Temporary Redirect\nlocation: http://localhost:";
					std::cout << this->_port;
					std::cout << "/forms/form_delete_failure.html\n\n";
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