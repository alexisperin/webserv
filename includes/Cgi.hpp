/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/05 15:35:48 by yhuberla          #+#    #+#             */
/*   Updated: 2023/06/07 13:22:17 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP

# include <string>
# include <iostream>
# include <unistd.h>
# include "utils.hpp"

class Server;

# define BUFFER_SIZE 30000

class Cgi {
	private:

	public:
		Cgi(std::string header, std::string file_path, Server *serv);
		~Cgi(void);
};

#endif
