/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 14:56:48 by yhuberla          #+#    #+#             */
/*   Updated: 2023/05/25 17:09:34 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Webserv.hpp"

Server::Server(void) : _body_size(1), _body_sighted(false)
{
}

Server::~Server(void)
{
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

static std::string read_data(std::ifstream &indata)
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

void Server::send_error(int socket_fd, int err_code, std::string errstr)
{
	std::cerr << " -- status return " << errstr << " --" << std::endl;
	std::string content = "HTTP/1.1 " + errstr + '\n';
	if (this->_error_map.find(err_code) != this->_error_map.end())
	{
		std::string file_abs_path = this->_root + this->_error_map[err_code];
		std::ifstream newdata(file_abs_path);
		if (!newdata.is_open())
			throw Webserv::InvalidFileException();
		std::string file_content = read_data(newdata);

		content += "Content-Type:text/html\nContent-Length: ";
		content += std::to_string(file_content.size()) + "\n\n" + file_content;
	}
	send(socket_fd, content.c_str(), content.size(), 0);
}

std::string Server::get_root_from_locations(std::string & loc, int head_offset, std::string method)
{
	std::string substr_loc = loc.substr(4 + head_offset, loc.find(" ", 4 + head_offset) - (4 + head_offset));
	size_t match_size = 0;
	size_t match_index;
	for (size_t loc_index = 0; loc_index < this->_locations.size(); loc_index++)
	{
		size_t loc_size = this->_locations[loc_index]->_location.size();
		if (!substr_loc.compare(0, loc_size, this->_locations[loc_index]->_location))
		{
			if (std::find(this->_locations[loc_index]->_methods.begin(), this->_locations[loc_index]->_methods.end(), method) != this->_locations[loc_index]->_methods.end())
			{
				if (!match_size || loc_size > match_size)
				{
					match_size = loc_size;
					match_index = loc_index;
				}
			}
		}
	}
	if (!match_size)
		return (this->_root);
	loc = loc.substr(0, 4 + head_offset) + loc.substr(4 + head_offset + match_size);
	return (this->_locations[match_index]->_root);
}

void Server::receive_put_content(int socket_fd, char buffer[30000], std::ofstream &outfile, size_t expected_size)
{
	std::string bufstr(buffer);
	std::cout << "bufstr size: " << bufstr.size() << std::endl;
	// std::cout << bufstr << std::endl;
	outfile << bufstr;

	if (bufstr.size() > this->_body_size * 1000000)
	{
		std::cerr << "file size exceeds max_body_size of " << this->_body_size << "MB" << std::endl;

		return (send_error(socket_fd, 413, "413 Payload Too Large"));
	}
	else if (bufstr.size() != expected_size)
		return (send_error(socket_fd, 412, "412 Precondition Failed"));

	std::string content("HTTP/1.1 200 OK\n");
	send(socket_fd, content.c_str(), content.size(), 0);
	std::cout << "response sent to " << socket_fd << ": " << content << std::endl;
}

void Server::analyse_request(int socket_fd, char buffer[30000])
{
	std::string bufstr(buffer);
	std::string content;
	std::cout << bufstr << std::endl;

	if (!bufstr.compare(0, 4, "GET ") || !bufstr.compare(0, 5, "HEAD "))
	{
		int head_offset = !bufstr.compare(0, 5, "HEAD ");
		std::cout << "GET DETECTED" << std::endl;
		std::string file_abs_path = get_root_from_locations(bufstr, head_offset, "GET");

		if (!bufstr.compare(4 + head_offset, 2, "/ "))
		{
			file_abs_path += this->_index_files.front();
		}
		else if (!bufstr.compare(4 + head_offset, 1, "/"))
		{
			file_abs_path += bufstr.substr(5 + head_offset, bufstr.find(" ", 5 + head_offset) - (5 + head_offset));
		}
		else
		{
			std::cerr << "TODO ?" << std::endl;
			return ;
		}

		std::cout << "reading from file: |" << file_abs_path << '|' << std::endl;
		std::ifstream indata(file_abs_path);
		std::string file_content;
		if (!indata.is_open())
		{
			content = ("HTTP/1.1 404 Not Found\nContent-Type:text/html\nContent-Length: ");
			file_abs_path = this->_root + this->_error_map[404];
			std::cout << "Error occured, now reading from file: |" << file_abs_path << '|' << std::endl;
			std::ifstream newdata(file_abs_path);
			if (!newdata.is_open())
				throw Webserv::InvalidFileException();
			file_content = read_data(newdata);
		}
		else
		{
			content = ("HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: ");
			file_content = read_data(indata);
		}
	
		content += std::to_string(file_content.size()) + "\n\n" + file_content;
		send(socket_fd, content.c_str(), content.size(), 0);
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
			return (send_error(socket_fd, 411, "411 Length Required"));
		std::istringstream iss(bufstr.substr(index + 16, bufstr.find('\n', index + 16)));
		size_t expected_size;
		iss >> expected_size;
		if (iss.fail() || expected_size > this->_body_size * 1000000)
			return (send_error(socket_fd, 412, "412 Precondition Failed"));

		std::string file_abs_path = this->_root;

		if (!bufstr.compare(4 + post_offset, 2, "/ "))
		{
			std::cerr << "Time to do some research boys, PUT / DOES exist." << std::endl;
			return ;
		}
		else if (!bufstr.compare(4 + post_offset, 1, "/"))
		{
			file_abs_path += bufstr.substr(5 + post_offset, bufstr.find(' ', 5 + post_offset) - (5 + post_offset));
		}
		else
		{
			std::cerr << "Time to do some research boys, PUT not/ DOES exist." << std::endl;
			return ;
		}

		std::cout << "checking if file: |" << file_abs_path << "| exists" << std::endl;
		std::ifstream indata(file_abs_path);
		if (!indata.is_open())
			content = "HTTP/1.1 201 Created\n";
		else
			content = "HTTP/1.1 100 Continue\n";
		if (!post_offset)
		{
			std::ofstream outdata(file_abs_path, std::ofstream::trunc);
			send(socket_fd, content.c_str(), content.size(), 0);
			recv(socket_fd, buffer, 30000, 0);
			receive_put_content(socket_fd, buffer, outdata, expected_size);
			outdata.close();
		}
		else
		{
			std::cout << "entering here" << std::endl;
			std::ofstream outdata(file_abs_path);
			send(socket_fd, content.c_str(), content.size(), 0);
			if (post_offset)
				return ;
			recv(socket_fd, buffer, 30000, 0);
			std::cout << "entering here too" << std::endl;
			receive_put_content(socket_fd, buffer, outdata, expected_size);
			outdata.close();
		}
		indata.close();
	}
	else if (!bufstr.compare(0, 7, "DELETE "))
	{
		std::cout << "DELETE DETECTED" << std::endl;
		std::string file_abs_path = this->_root;
		if (!bufstr.compare(7, 2, "/ "))
			content = "HTTP/1.1 400 Bad Request\n"; // To discuss ???
		else if (!bufstr.compare(7, 1, "/"))
		{
			file_abs_path += bufstr.substr(8, bufstr.find(" ", 8) - 8);
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
		}
		else
			content = "HTTP/1.1 400 Bad Request\n"; // To discuss ???
		send(socket_fd, content.c_str(), content.size(), 0);
	}
}

// ************************************************************************** //
//                                  Public                                    //
// ************************************************************************** //

static int is_error_code(int code)
{
	return ((code >= 400 && code <= 418) || (code >= 421 && code <= 426) || (code >= 428 && code <= 431) || code == 451
		|| (code >= 500 && code <= 508) || (code >= 510 && code <= 511));
}

void Server::compare_block_info(std::string line, std::ifstream & indata)
{
	std::cout << "line: " << line << std::endl;
	if (!line.compare (0, 9, "location "))
		this->_locations.push_back(new Location(line, indata));
	else if (line.back() != ';' || line.find(';') != line.size() - 1)
		throw Webserv::InvalidFileContentException();
	else if (line[0] == '#')
		;
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
		std::ifstream newdata(file_abs_path);
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
	if (this->_index_files.empty() || this->_ports.empty() || this->_root.empty())
		throw Server::IncompleteServerException();
	if (this->_error_map.find(404) == this->_error_map.end())
	{
		std::string err404("error_files/404.html");
		std::string file_abs_path = this->_root + err404;
		std::ifstream newdata(file_abs_path);
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
			if (this->_locations[index]->_location == this->_locations[sub_index]->_location)
				throw Webserv::InvalidFileContentException();
		}
		if (!this->_locations[index]->_return.empty())
		{
			for (size_t loc_index = 0; loc_index < index; loc_index++)
			{
				if (this->_locations[loc_index]->_location == this->_locations[index]->_return)
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
		all_ports.insert(*it);
}

void Server::setup_server(void)
{
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

	int new_socket, ready;
	size_t addrlen;
	ssize_t valread;

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
				if ((new_socket = accept(pfds[index].fd, (struct sockaddr *) &address[index], (socklen_t*) &addrlen)) < 0)
				{
					perror("accept");
					return ;
				}

				char buffer[30000] = {0};
				valread = recv(new_socket, buffer, 30000, 0);
				analyse_request(new_socket, buffer);
				std::cout << "------------------content message sent to " << new_socket << "-------------------\n";
				close(new_socket);
	
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
