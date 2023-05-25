/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 14:52:49 by yhuberla          #+#    #+#             */
/*   Updated: 2023/05/25 17:45:28 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <list>
# include <vector>
# include <map>
# include <set>
# include <string>
# include <exception>
# include <iostream>
# include <sys/wait.h>
# include <poll.h>
# include "Location.hpp"
class Location;

class Server
{
	private:
		std::list<int> _ports;
		std::string _server_type; // listen 80 <type=ssl,default_server>;
		std::list<std::string> _server_names; // server_name bla.com;
		std::string _root;
		std::list<std::string> _index_files;
		size_t _body_size;
		bool _body_sighted;
		std::map<int, std::string> _error_map;
		std::vector<Location *> _locations;

		void analyse_request(int socket_fd, char buffer[30000]);
		void receive_put_content(int socket_fd, char buffer[30000], std::ofstream &outfile, size_t expected_size);
		void send_error(int socket_fd, int err_code, std::string errstr);
		std::string get_path_from_locations(std::string & loc, int head_offset, std::string method);

	public:
		Server(void);
		~Server(void);

		void check_set_default(void);
		void display_serv_content(void);
		void setup_server(void);
		void waitup_server(void);
		void compare_block_info(std::string line, std::ifstream & indata);
		void add_ports(std::set<int> &all_ports, size_t *number_of_ports);

	class IncompleteServerException : public std::exception
	{
		public:
			const char *what() const throw();
	};
};

#endif
