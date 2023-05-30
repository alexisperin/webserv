/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/17 10:12:47 by aperin            #+#    #+#             */
/*   Updated: 2023/05/30 12:36:21 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <unistd.h>

# include <sstream>
# include <sys/socket.h>
# include <netinet/in.h>

# include "Server.hpp"
class Server;
# include "utils.hpp"

class Webserv
{
	private:
		std::list<Server *> _servers;

	public:
		Webserv(void);
		~Webserv(void);

		void init(std::string file_name);
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

	class MissingDefault404Exception : public std::exception
	{
		public:
			const char *what() const throw();
	};

	class SystemCallException : public std::exception
	{
		public:
			const char *what() const throw();
	};
};

#endif
