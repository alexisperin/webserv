/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/05 15:35:48 by yhuberla          #+#    #+#             */
/*   Updated: 2023/06/05 15:42:43 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP

# include <string>
# include <iostream>

class Server;

class Cgi {
	private:

	public:
		Cgi(std::string header, std::string file_path, Server *serv);
		~Cgi(void);
};

#endif
