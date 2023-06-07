/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 14:52:49 by yhuberla          #+#    #+#             */
/*   Updated: 2023/06/05 15:41:02 by yhuberla         ###   ########.fr       */
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
# include "Location.hpp"
# include "Cgi.hpp"
class Location;

# define BUFFER_SIZE 30000

class Server
{
	private:
		int _socket_fd;
		std::list<int> _ports;
		std::string _server_type; // listen 80 <type=ssl,default_server>;
		std::list<std::string> _server_names; // server_name bla.com;
		std::string _root;
		std::list<std::string> _index_files;
		size_t _body_size;
		size_t _current_body_size;
		bool _body_sighted;
		std::map<int, std::string> _error_map;
		std::vector<Location *> _locations;

		void analyse_request(std::string bufstr);
		void receive_put_content(std::string bufstr, std::ofstream &outfile, size_t expected_size, std::string content);
		std::string check_chunck_encoding(std::string bufstr);
		void send_message(std::string msg);
		void send_error(int err_code, std::string errstr);
		void send_method_error(std::vector<std::string> methods);
		std::string recv_lines(int check_header);
		std::string get_path_from_locations(std::string & loc, int head_offset, std::string method);
		std::string get_first_index_file(std::string root, std::string prev_loc, std::list<std::string> index_files, bool auto_index);

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
