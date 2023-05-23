/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/17 10:12:47 by aperin            #+#    #+#             */
/*   Updated: 2023/05/23 17:18:41 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <unistd.h>

# include <fstream>
# include <sstream>
# include <sys/socket.h>
# include <netinet/in.h>

# include "Server.hpp"

class Webserv
{
	private:
		std::list<Server *> _servers;

	public:
		Webserv(std::string file_name);
		~Webserv(void);

		void display_servs_content(void);
		void setup_servers(void);

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

	class DuplicatePortsException : public std::exception
	{
		public:
			const char *what() const throw();
	};
};

#endif
