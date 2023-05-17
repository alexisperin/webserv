/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aperin <aperin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/17 10:12:47 by aperin            #+#    #+#             */
/*   Updated: 2023/05/17 15:53:52 by aperin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <unistd.h>

# include <fstream>
# include <sstream>
# include <sys/socket.h>
# include <netinet/in.h>

# include <list>
# include <map>
# include <string>
# include <exception>
# include <iostream>

class Webserv
{
	private:
		std::list<int> _ports;
		std::string _server_type; // listen 80 <type=ssl,default_server>;
		std::list<std::string> _server_names; // server_name bla.com;
		std::string _root;
		std::list<std::string> _index_files;
		int _body_size;
		std::map<int, std::string> _error_map;
		void compare_block_info(std::string line);
		
	public:
		Webserv(std::string file_name);
		~Webserv(void);

		void display_serv_content(void);
		void setup_server(void);
	
	class InvalidFileException : public std::exception
	{
		public:
			const char *what() const throw();
	};
	
	class InvalidFileExtensionException : public std::exception
	{
		public:
			const char *what() const throw();
	};
	
	class InvalidFileContentException : public std::exception
	{
		public:
			const char *what() const throw();
	};
};

#endif
