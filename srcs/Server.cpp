/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 14:56:48 by yhuberla          #+#    #+#             */
/*   Updated: 2023/06/07 15:48:09 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Webserv.hpp"

Server::Server(void) : _body_size(1), _body_sighted(false)
{
}

Server::~Server(void)
{
	// std::cout << "Destructor of Server called." << std::endl;
	this->_ports.clear();
	this->_server_names.clear();
	this->_index_files.clear();
	this->_error_map.clear();
	std::vector<Location *>::iterator it = this->_locations.begin();
	std::vector<Location *>::iterator ite = this->_locations.end();
	for (; it != ite; it++)
		delete *it;
	this->_locations.clear();
}

// ************************************************************************** //
//                                  Private                                   //
// ************************************************************************** //

void Server::send_message(std::string msg)
{
	std::cerr << std::endl << " -- status return " << msg << " --" << std::endl;
	std::string content = "HTTP/1.1 " + msg + "\n\n";
	send(this->_socket_fd, content.c_str(), content.size(), 0);
}

void Server::send_error(int err_code, std::string errstr)
{
	std::cerr << std::endl << " -- status return " << errstr << " --" << std::endl;
	std::string content = "HTTP/1.1 " + errstr + '\n';
	if (this->_error_map.find(err_code) != this->_error_map.end())
	{
		std::string file_abs_path = this->_root + this->_error_map[err_code];
		std::ifstream newdata(file_abs_path.c_str());
		if (!newdata.is_open())
			goto SEND;
		std::string file_content = read_data(newdata);
		std::ostringstream content_length;

		content_length << file_content.size();
		content += "Content-Type:text/html\nContent-Length: ";
		content += content_length.str() + "\n\n" + file_content;
		newdata.close();
	}
	else
		content += '\n';
	SEND:
	send(this->_socket_fd, content.c_str(), content.size(), 0);
	throw Webserv::QuickerReturnException();
}

void Server::send_method_error(std::vector<std::string> methods)
{
	std::string content = "405 Method Not Allowed\nAllow: ";
	std::vector<std::string>::iterator it = methods.begin();
	std::vector<std::string>::iterator ite = methods.end();
	for (; it != ite; it++)
	{
		content += *it;
		if (++it != ite)
			content += ", ";
		--it;
	}
	send_error(405, content);
}

std::string Server::get_first_index_file(std::string root, std::string prev_loc, std::list<std::string> index_files, bool auto_index)
{
	std::list<std::string>::iterator it = index_files.begin();
	std::list<std::string>::iterator ite = index_files.end();
	for (; it != ite; it++)
	{
		std::string test_path = root + *it;
		std::ifstream indata(test_path.c_str());
		if (indata.is_open())
			return (root + *it);
	}
	if (auto_index)
	{
		std::string listing = "listing.tmp.html";
		std::string file_path = root + listing;
		std::ofstream outdata(file_path.c_str(), std::ofstream::trunc);
		outdata << "<!DOCTYPE html>\n<html>\n <body>\n	<div>\n		<H1>Index of " << prev_loc << "</H1>\n	</div>\n";
		struct dirent *dent;
		DIR *dir = opendir(root.c_str());
		std::string dot = ".";
		if (dir != NULL)
		{
			while ((dent = readdir(dir)) != NULL)
			{
				if (dot.compare(0, 2, dent->d_name) && listing.compare(0, 16, dent->d_name))
					outdata << "<p><a href=\"" << prev_loc << '/' << dent->d_name << "\">" << dent->d_name << "</a></p>\n";
			}
		}
		else
			send_error(404, "404 Not Found");
		closedir(dir);
		outdata << " </body>\n</html>";
		outdata.close();
		return (root + listing);
	}
	send_error(404, "404 Not Found");
	return ("");
}

std::string Server::check_chunck_encoding(std::string bufstr)
{
	if (bufstr.find("Transfer-Encoding: chunked") == std::string::npos)
		return  (bufstr);
	std::string sub_bufstr;
	char buffer[BUFFER_SIZE + 1] = {0};
	// send_error(this->_socket_fd, 100, "100 Continue");
	ssize_t valread = recv(this->_socket_fd, buffer, BUFFER_SIZE, 0);
	if (valread == -1)
		send_error(500, "500 Internal Server Error");
	while (valread && buffer[0] != '0')
	{
		bufstr += buffer;
		display_special_characters(buffer);
		// send_error(this->_socket_fd, 100, "100 Continue");
		valread = recv(this->_socket_fd, buffer, BUFFER_SIZE, 0);
		if (valread == -1)
			send_error(500, "500 Internal Server Error");
		buffer[valread] = '\0';
		std::cout << "first char of buffer: |" << buffer[0] << '|' << std::endl;
	}
	return (bufstr);
}

/* look for "/cgi/" in file_path and call cgi if found */
void Server::check_for_cgi(std::string header, std::string file_path, int method_offset, std::string method)
{
	size_t search = file_path.find("/cgi/");
	if (search == std::string::npos)
		return ;
	// std::cout << "/cgi/ found in path" << std::endl;
	// std::cout << std::endl;
	// display_special_characters(header);
	// std::cout << std::endl << "cgi_path: " << file_path << std::endl;

	size_t end = file_path.find('/', search + 5);
	if (end == std::string::npos)
		header = header.substr(0, method_offset) + '/' + header.substr(method_offset + file_path.size());
	else
	{
		file_path = file_path.substr(0, end);
		header = header.substr(0, method_offset) + header.substr(method_offset + end);
	}
	get_path_from_locations(header, method_offset - 4, method, 1);
	Cgi(header, file_path, this);
}

std::string Server::recv_lines(int check_header)
{
	std::string bufstr;
	char buffer[BUFFER_SIZE + 1] = {0};
	ssize_t valread = recv(this->_socket_fd, buffer, BUFFER_SIZE, 0);
	if (valread == -1)
		send_error(500, "500 Internal Server Error");
	bufstr += buffer;
	while (valread == BUFFER_SIZE) //TODO what if request of exactly BUFFER_SIZE bytes
	{
		send_message("100 Continue");
		valread = recv(this->_socket_fd, buffer, BUFFER_SIZE, 0);
		if (valread == -1)
			send_error(500, "500 Internal Server Error");
		else if (valread)
		{
			buffer[valread] = '\0';
			bufstr += buffer;
		}
		// std::cout << "valread: " << valread << std::endl;
	}
	if (check_header)
		bufstr = check_chunck_encoding(bufstr);
	return (bufstr);
}

std::string Server::get_path_from_locations(std::string & loc, int method_offset, std::string method, bool recursive_stop)
{
	// std::cout << "get_path_from_loc" << std::endl;
	std::string ret;
	std::string substr_loc = loc.substr(4 + method_offset, loc.find(" ", 4 + method_offset) - (4 + method_offset));
	size_t match_size = 0;
	size_t match_index;
	bool auto_index;
	std::list<std::string> match_index_files;
	for (size_t loc_index = 0; loc_index < this->_locations.size(); loc_index++)
	{
		size_t loc_size = this->_locations[loc_index]->location.size();
		// std::cout << "match_index: " << match_index << std::endl;
		// std::cout << "loc_size: " << loc_size << ", substr_loc.size" << substr_loc.size() << std::endl;
		// std::cout << "loc: " << this->_locations[loc_index]->location << ", substr_loc" << substr_loc << std::endl;
		if (loc_size <= substr_loc.size() && ((!this->_locations[loc_index]->suffixed && !substr_loc.compare(0, loc_size, this->_locations[loc_index]->location))
			|| (this->_locations[loc_index]->suffixed && !substr_loc.compare(substr_loc.size() - loc_size, loc_size, this->_locations[loc_index]->location))))
		{
			if (!match_size || loc_size > match_size || this->_locations[loc_index]->suffixed)
			{
				match_size = loc_size;
				match_index = loc_index;
				match_index_files = this->_locations[loc_index]->index_files;
				auto_index = this->_locations[loc_index]->auto_index;
				this->_current_body_size = this->_locations[loc_index]->body_size;
				if (this->_locations[loc_index]->suffixed)
				{
					match_size = 1;
					break ;
				}
			}
		}
	}
	// std::cout << "match_size: " << match_size << std::endl;
	if (!recursive_stop && match_size && std::find(this->_locations[match_index]->methods.begin(), this->_locations[match_index]->methods.end(), method) == this->_locations[match_index]->methods.end())
	{
		send_method_error(this->_locations[match_index]->methods);
		throw Webserv::QuickerReturnException();
	}
	if (this->_current_body_size == std::string::npos)
		this->_current_body_size = this->_body_size;
	if (!match_size)
		ret = this->_root;
	else
		ret = this->_locations[match_index]->root;
	std::string prev_loc = loc.substr(4 + method_offset, match_size);
	loc = loc.substr(0, 4 + method_offset) + loc.substr(4 + method_offset + match_size);
	// std::cout << "loc after: " << loc << std::endl;

	if (!loc.compare(4 + method_offset, 2, "/ ") || !loc.compare(4 + method_offset, 1, " "))
	{
		if (match_index_files.empty())
			ret = get_first_index_file(ret, prev_loc, this->_index_files, auto_index);
		else
			ret = get_first_index_file(ret, prev_loc, match_index_files, auto_index);
	}
	else if (!loc.compare(4 + method_offset, 1, "/"))
		ret += loc.substr(5 + method_offset, loc.find(" ", 5 + method_offset) - (5 + method_offset));
	else
		ret += loc.substr(4 + method_offset, loc.find(" ", 4 + method_offset) - (4 + method_offset));
	loc = loc.substr(0, 4 + method_offset) + ret + loc.substr(loc.find(' ', 4 + method_offset));
	if (recursive_stop)
		return ("");
	check_for_cgi(loc, ret, 4 + method_offset, method);
	if (match_size && !this->_locations[match_index]->cgi.empty())
		Cgi(loc, this->_locations[match_index]->root + this->_locations[match_index]->cgi, this);
	return (ret);
}

void Server::receive_put_content(std::string bufstr, std::ofstream &outfile, size_t expected_size, std::string content)
{
	std::cout << "bufstr size: " << bufstr.size() << ", selected max_body_size: " << this->_current_body_size << std::endl;
	// std::cout << bufstr << std::endl;

	if (bufstr.size() > this->_current_body_size * 1000000)
	{
		std::cerr << "file size exceeds max_body_size of " << this->_current_body_size << "MB" << std::endl;
		send_error(413, "413 Payload Too Large");
	}
	else if (bufstr.size() != expected_size)
		send_error(412, "412 Precondition Failed");

	outfile << bufstr;
	send_message(content);
}

void Server::analyse_request(std::string bufstr)
{
	if (bufstr.empty())
	{
		std::cout << " -- empty request --" << std::endl;
		return ;
	}
	std::string content;
	// std::cout << bufstr.size() << ": " << bufstr << std::endl;
	display_special_characters(bufstr); //used for debug because /r present in buffer

	if (check_http_version(bufstr))
		send_error(505, "505 HTTP Version Not Supported");
	if (check_correct_host(bufstr) || check_header_names(bufstr))
		send_error(400, "400 Bad Request");
	if (!bufstr.compare(0, 4, "GET ") || !bufstr.compare(0, 5, "HEAD "))
	{
		int head_offset = !bufstr.compare(0, 5, "HEAD ");
		std::cout << "GET DETECTED" << std::endl;
		std::string file_abs_path = get_path_from_locations(bufstr, head_offset, "GET", 0);

		std::cout << "reading from file: |" << file_abs_path << '|' << std::endl;
		std::ifstream indata(file_abs_path.c_str());
		if (!indata.is_open())
			send_error(404, "404 Not Found");

		content = "HTTP/1.1 200 OK\nContent-Type:" + GET_content_type(file_abs_path) + "\nContent-Length: ";
		std::string file_content = read_data(indata);

		if (file_abs_path.size() >= 16 && !file_abs_path.compare(file_abs_path.size() - 16, 16, "listing.tmp.html"))
			std::remove(file_abs_path.c_str());

		std::ostringstream content_length;
		content_length << file_content.size();
		content += content_length.str() + "\n\n" + file_content;
		send(this->_socket_fd, content.c_str(), content.size(), 0);
	}
	else if (!bufstr.compare(0, 4, "PUT ") || !bufstr.compare(0, 5, "POST "))
	{
		int post_offset = !bufstr.compare(0, 5, "POST ");

		if (!post_offset)
			std::cout << "PUT DETECTED" << std::endl;
		else
			std::cout << "POST DETECTED" << std::endl;

		std::string line;
		size_t index = bufstr.find("Content-Length: ");
		if (index == std::string::npos)
			send_error(411, "411 Length Required");
			// send_error(201, "201 Created");
		std::istringstream iss(bufstr.substr(index + 16, bufstr.find('\n', index + 16)));
		size_t expected_size;
		iss >> expected_size;
		if (iss.fail() || expected_size > this->_current_body_size * 1000000)
			send_error(412, "412 Precondition Failed");

		std::string file_abs_path;
		if (!post_offset)
			file_abs_path = get_path_from_locations(bufstr, 0, "PUT", 0);
		else
			file_abs_path = get_path_from_locations(bufstr, 1, "POST", 0);

		std::cout << "checking if file: |" << file_abs_path << "| exists" << std::endl;
		std::ifstream indata(file_abs_path.c_str());
		if (!indata.is_open())
			content = "201 Created";
		else
			content = "200 OK";
		indata.close();
		if (!post_offset)
		{
			std::ofstream outdata(file_abs_path.c_str(), std::ofstream::trunc);
			if (!outdata.is_open())
				send_error(404, "404 Not Found");
			send_message("100 Continue");
			if (expected_size)
			{
				bufstr = recv_lines(0);
				receive_put_content(bufstr, outdata, expected_size, content);
			}
			outdata.close();
		}
		else // post should use cgi_script
		{
			std::string body = get_body(bufstr);
			if (body.size() != expected_size)
				send_error(412, "412 Precondition Failed");
			else if (expected_size > this->_current_body_size * 1000000)
			{
				std::cerr << "file size exceeds max_body_size of " << this->_current_body_size << "MB" << std::endl;
				send_error(413, "413 Payload Too Large");
			}
			run_script(this->_socket_fd, body);
			// content = "GET / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: curl/7.64.1\r\nAccept: */*\r\n\r\n";
			// send(this->_socket_fd, content.c_str(), content.size(), 0);
			// std::cout << "entering here" << std::endl;
			// std::ofstream outdata(file_abs_path.c_str());
			// send(this->_socket_fd, content.c_str(), content.size(), 0);
			// if (!bufstr.find("\r\n\r\n"))
			// 	bufstr = recv_lines(socket_fd, 0);
			// std::cout << "post post: " << bufstr << std::endl;
			// receive_put_content(socket_fd, bufstr, outdata, expected_size);
			// outdata.close();
		}
	}
	else if (!bufstr.compare(0, 7, "DELETE "))
	{
		std::cout << "DELETE DETECTED" << std::endl;
		std::string file_abs_path = get_path_from_locations(bufstr, 3, "DELETE", 0);
		std::cout << "Wants to delete file " << file_abs_path << std::endl;
		if (!std::remove(file_abs_path.c_str()))
		{
			std::cout << "File deleted\n";
			content = "HTTP/1.1 204 No Content\n";
		}
		else
		{
			std::cout << "File could not be deleted\n";
			content = "HTTP/1.1 404 Not Found\n";
		}
		send(this->_socket_fd, content.c_str(), content.size(), 0);
	}
}

// ************************************************************************** //
//                                  Public                                    //
// ************************************************************************** //

void Server::compare_block_info(std::string line, std::ifstream & indata)
{
	std::cout << "line: " << line << std::endl;
	if (!line.compare (0, 9, "location "))
		this->_locations.push_back(new Location(line, indata, this->_root));
	else if (line[0] == '#')
		;
	else if (line[line.size() - 1] != ';' || line.find(';') != line.size() - 1)
		throw Webserv::InvalidFileContentException();
	else if (!line.compare(0, 7, "listen "))
	{
		std::istringstream iss(line.substr(7));
		int toint;
		iss >> toint;
		if (iss.fail() || !toint)
			throw Webserv::InvalidFileContentException();
		this->_ports.push_back(toint);
		size_t index = 7;
		while (std::isdigit(line[index]))
			++index;
		if (index == line.size() - 1 || (index == line.size() - 2 && line[index] == ' '))
			return ;
		if (line.compare(index, 15, " default_server"))
			throw Webserv::InvalidFileContentException();
		this->_server_type = "default_server";
	}
	else if (!line.compare(0, 12, "server_name "))
	{
		line = line.substr(12);
		if (line.empty() || !line.compare(";"))
			throw Webserv::InvalidFileContentException();
		while (!line.empty())
		{
			size_t size = line.find(' ');
			if (size == std::string::npos)
			{
				if (line.compare(";"))
					this->_server_names.push_back(line.substr(0, line.size() - 1));
				line = "";
			}
			else
			{
				this->_server_names.push_back(line.substr(0, size));
				line = line.substr(size + 1);
			}
		}
	}
	else if (!line.compare(0, 5, "root "))
	{
		this->_root = line.substr(5, line.size() - 6 - (line[line.size() - 2] == ' '));
		if (this->_root.find(' ') != std::string::npos || !this->_root.compare(";"))
			throw Webserv::InvalidFileContentException();
	}
	else if (!line.compare(0, 6, "index "))
	{
		line = line.substr(6);
		if (line.empty() || !line.compare(";"))
			throw Webserv::InvalidFileContentException();
		while (!line.empty())
		{
			size_t size = line.find(' ');
			if (size == std::string::npos)
			{
				if (line.compare(";"))
					this->_index_files.push_back(line.substr(0, line.size() - 1));
				line = "";
			}
			else
			{
				this->_index_files.push_back(line.substr(0, size));
				line = line.substr(size + 1);
			}
		}
	}
	else if (!line.compare("client_max_body_size 0;") || !line.compare("client_max_body_size 0 ;"))
	{
		if (this->_body_sighted)
			throw Webserv::InvalidFileContentException();
		this->_body_sighted = true;
		this->_body_size = 0;
	}
	else if (!line.compare(0, 21, "client_max_body_size "))
	{
		if (this->_body_sighted)
			throw Webserv::InvalidFileContentException();
		this->_body_sighted = true;
		std::istringstream iss(line.substr(21));
		int toint;
		iss >> toint;
		if (iss.fail() || !toint)
			throw Webserv::InvalidFileContentException();
		this->_body_size = toint;
		size_t index = 21;
		while (std::isdigit(line[index]))
			++index;
		if ((index == line.size() - 2 && line[index] == 'M') || (index == line.size() - 3 && line[index] == 'M' && line[index + 1] == ' '))
			return ;
		throw Webserv::InvalidFileContentException();
	}
	else if (!line.compare(0, 11, "error_page "))
	{
		std::list<int> toints;
		size_t index = 11;
		
		while (std::isdigit(line[index]))
		{
			std::istringstream iss(line.substr(index));
			int toint;
			iss >> toint;
			if (iss.fail())
				throw Webserv::InvalidFileContentException();
			if (!is_error_code(toint))
				break ;
			toints.push_back(toint);
			while (std::isdigit(line[index]))
				++index;
			if (line[index] != ' ')
				throw Webserv::InvalidFileContentException();
			++index;
		}
		if (index == 11)
			throw Webserv::InvalidFileContentException();
		std::string value = line.substr(index, line.size() - index - 1 - (line[line.size() - 2] == ' '));
		if (value.find(' ') != std::string::npos)
			throw Webserv::InvalidFileContentException();
		std::string file_abs_path = this->_root + value;
		std::ifstream newdata(file_abs_path.c_str());
		if (!newdata.is_open())
			throw Webserv::InvalidFileContentException();
		newdata.close();
		std::list<int>::iterator it = toints.begin();
		std::list<int>::iterator ite = toints.end();
		for (; it != ite; it++)
			this->_error_map.insert(std::pair<int,std::string>(*it, value));
	}
}

void Server::check_set_default(void)
{
	if (this->_ports.empty() || this->_root.empty())
		throw Server::IncompleteServerException();
	if (this->_error_map.find(404) == this->_error_map.end())
	{
		std::string err404("error_files/404.html");
		std::string file_abs_path = this->_root + err404;
		std::ifstream newdata(file_abs_path.c_str());
		if (!newdata.is_open())
			throw Webserv::MissingDefault404Exception();
		newdata.close();
		this->_error_map.insert(std::pair<int,std::string>(404, err404));
	}
	if (this->_server_type.empty())
		this->_server_type = "default_server";
	for (size_t index = 0; index < this->_locations.size(); index ++)
	{
		for (size_t sub_index = 0; sub_index < index; sub_index++)
		{
			if (this->_locations[index]->location == this->_locations[sub_index]->location)
				throw Webserv::InvalidFileContentException();
		}
		if (!this->_locations[index]->_return.empty())
		{
			for (size_t loc_index = 0; loc_index < index; loc_index++)
			{
				if (this->_locations[loc_index]->location == this->_locations[index]->_return)
					*this->_locations[index] = *this->_locations[loc_index];
			}
			if (!this->_locations[index]->_return.empty())
				throw Webserv::InvalidFileContentException();
		}
	}
}

void Server::display_serv_content(void)
{
	std::cout << "Server content:" << std::endl;
	std::cout << "\t-ports: ";
	std::list<int>::iterator iit = this->_ports.begin();
	std::list<int>::iterator iite = this->_ports.end();
	for (; iit != iite; iit++)
		std::cout << *iit << ' ';
	std::cout << std::endl;
	std::cout << "\t-server_type: " << this->_server_type << std::endl;
	std::cout << "\t-server_names: ";
	std::list<std::string>::iterator it = this->_server_names.begin();
	std::list<std::string>::iterator ite = this->_server_names.end();
	for (; it != ite; it++)
		std::cout << *it << ' ';
	std::cout << std::endl;
	std::cout << "\t-root: " << this->_root << std::endl;
	std::cout << "\t-index: ";
	it = this->_index_files.begin();
	ite = this->_index_files.end();
	for (; it != ite; it++)
		std::cout << *it << ' ';
	std::cout << std::endl;
	std::cout << "\t-body_size: " << this->_body_size << std::endl;
	std::cout << "\t-error_codes: ";
	std::map<int, std::string>::iterator iiit = this->_error_map.begin();
	std::map<int, std::string>::iterator iiite = this->_error_map.end();
	for (; iiit != iiite; iiit++)
		std::cout << iiit->first << ' ' << iiit->second << ' ';
	std::cout << std::endl;
	std::vector<Location *>::iterator iiiit = this->_locations.begin();
	std::vector<Location *>::iterator iiiite = this->_locations.end();
	for (; iiiit != iiiite; iiiit++)
		(*iiiit)->display_loc_content();
}

void Server::add_ports(std::set<int> &all_ports, size_t *number_of_ports)
{
	*number_of_ports += this->_ports.size();
	std::list<int>::iterator it = this->_ports.begin();
	std::list<int>::iterator ite = this->_ports.end();
	for (; it != ite; it++)
	{
		if (all_ports.find(*it) != all_ports.end())
			it = this->_ports.erase(it);
		all_ports.insert(*it);
	}
}

void Server::setup_server(void)
{
	if (this->_ports.empty())
		return ;
	struct pollfd pfds[this->_ports.size()];
	struct sockaddr_in address[this->_ports.size()];

	std::list<int>::iterator it = this->_ports.begin();
	std::list<int>::iterator ite = this->_ports.end();
	
	int index = 0;
	for (; it != ite; it++)
	{
		std::cout << "Setting up port " << *it << '.' << std::endl;

		if ((pfds[index].fd = socket(PF_INET, SOCK_STREAM, 0)) == 0)
		{
			perror("socket");
			return ;
		}
		pfds[index].events = POLLIN;

		address[index].sin_family = PF_INET;
		address[index].sin_addr.s_addr = INADDR_ANY;
		address[index].sin_port = htons(*it);

		if (bind(pfds[index].fd, (struct sockaddr *) &address[index], sizeof(address[index])) < 0)
		{
			perror("bind");
			return ;
		}

		if (listen(pfds[index].fd, 10) < 0)
		{
			perror("listen");
			return ;
		}
		++index;
	}

	int ready;
	size_t addrlen;

	while(1)
	{
		std::cout << "\n+++++++ Waiting for new connection ++++++++\n\n";
		ready = poll(pfds, this->_ports.size(), -1);
		if (ready == -1)
		{
			perror("poll");
			return ;
		}

		(ready == 1)
			? std::cout << "1 socket ready" << std::endl
			: std::cout << ready << " sockets ready" << std::endl;
		for (size_t index = 0; index < this->_ports.size(); index++) {

			if (pfds[index].revents != 0) {
				std::cout << "  - socket = " << pfds[index].fd << "; events: ";
				(pfds[index].revents & POLLIN)  ? std::cout << "POLLIN "  : std::cout << "";
				(pfds[index].revents & POLLHUP) ? std::cout << "POLLHUP " : std::cout << "";
				(pfds[index].revents & POLLERR) ? std::cout << "POLLERR " : std::cout << "";
				std::cout << std::endl << std::endl;

				addrlen = sizeof(address[index]);
				if ((this->_socket_fd = accept(pfds[index].fd, (struct sockaddr *) &address[index], (socklen_t*) &addrlen)) < 0)
				{
					perror("accept");
					return ;
				}

				try
				{
					std::string bufstr = recv_lines(1);
					analyse_request(bufstr);
				}
				catch (std::exception & e) {/*std::cerr << e.what() << std::endl;*/}
				std::cout << std::endl << "------------------content message sent to " << this->_socket_fd << "-------------------\n";
				close(this->_socket_fd);
	
				// if (pfds[j].revents & POLLIN) {
				// 	ssize_t s = read(pfds[j].fd, buf, sizeof(buf));
				// 	if (s == -1)
				// 		errExit("read");
				// 	printf("    read %zd bytes: %.*s\n",
				// 			s, (int) s, buf);
				// } else {                /* POLLERR | POLLHUP */
				// 	printf("    closing fd %d\n", pfds[j].fd);
				// 	if (close(pfds[j].fd) == -1)
				// 		errExit("close");
				// 	num_open_fds--;
				// }
			}
		}

	}
}

void Server::waitup_server(void)
{
	std::list<int>::iterator it = this->_ports.begin();
	std::list<int>::iterator ite = this->_ports.end();

	for (; it != ite; it++)
		waitpid(-1, NULL, 0);
}

// ************************************************************************** //
//                                 Exceptions                                 //
// ************************************************************************** //

const char* Server::IncompleteServerException::what() const throw()
{
	return ("[Server::IncompleteServerException] Missing line in server block.");
}
