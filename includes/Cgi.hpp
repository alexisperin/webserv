/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aperin <aperin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/05 15:35:48 by yhuberla          #+#    #+#             */
/*   Updated: 2023/06/15 13:41:35 by aperin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP

# include <string>
# include <iostream>
# include <unistd.h>
# include <map>
# include "utils.hpp"
# include <sys/stat.h>
# include <fcntl.h>

class Server;

# define BUFFER_SIZE 30000

class Cgi {
	private:
		std::string _header;
		std::string _file_path;
		Server *_serv;

		char **get_execve_args(void);
		char **set_envp(std::string saved_root);
		std::string get_port(void);
		std::string get_method(void);
		void add_path_info(std::map<std::string, std::string> & env_map, std::string root);
		std::string get_script_relative(std::string root);
		std::string get_remote_host(void);
		void add_server_names(std::map<std::string, std::string> & env_map);
		void add_header_field(std::map<std::string, std::string> & env_map, std::string key, std::string header_key);

	public:
		Cgi(std::string header, std::string file_path, Server *serv, std::string saved_root);
		~Cgi(void);
};

#endif
