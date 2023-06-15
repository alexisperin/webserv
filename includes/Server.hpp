/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aperin <aperin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 14:52:49 by yhuberla          #+#    #+#             */
/*   Updated: 2023/06/15 14:23:44 by aperin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <list>
# include <vector>
# include <map>
# include <set>
# include <exception>
# include <sys/wait.h>
# include <poll.h>
# include <dirent.h>
# include "Location.hpp"
# include "Cgi.hpp"
class Location;

class Server
{
	private:
		std::string _server_type;
		std::string _root;
		std::list<std::string> _index_files;
		size_t _body_size;
		bool _body_sighted;
		std::map<int, std::string> _error_map;
		std::vector<Location *> _locations;
		std::string _initial_loc;

		void check_chunck_encoding(std::string & bufstr);
		void check_for_cgi(std::string header, std::string bufstr, int method_offset, std::string method, std::string saved_root);
		void send_method_error(std::vector<std::string> methods);
		std::string get_path_from_locations(std::string & loc, int head_offset, std::string method, bool recursive_stop);
		void dir_listing(DIR *dir);
		std::string get_first_index_file(std::string root, std::list<std::string> index_files, bool auto_index);

	public:
		Server(void);
		~Server(void);

		int _socket_fd;
		size_t _current_body_size;
		std::list<int> _ports;
		std::list<std::string> _server_names;
		void check_set_default(void);
		void display_serv_content(void);
		void recv_lines(std::string & bufstr, int check_header);
		void analyse_request(std::string bufstr);
		void send_message(std::string msg);
		void send_error(int err_code, std::string errstr);
		void compare_block_info(std::string line, std::ifstream & indata);
		void add_ports(std::set<int> &all_ports, size_t *number_of_ports);

	class IncompleteServerException : public std::exception
	{
		public:
			const char *what() const throw();
	};
};

#endif
