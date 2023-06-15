/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aperin <aperin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/12 11:22:56 by aperin            #+#    #+#             */
/*   Updated: 2023/06/15 14:32:17 by aperin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>

#include "Webserv.hpp"

int main(int ac, char **av)
{
	std::string file_name;
	if (ac == 1)
		file_name = "conf/default.conf";
	else if (ac != 2) {
		std::cerr << "Usage: ./webserv <file.conf>" << std::endl;
		return (1);
	}
	else
		file_name = av[1];
	Webserv *my_serv = new Webserv();
	try
	{
		my_serv->init(file_name);
		my_serv->display_servs_content();
		my_serv->setup_servers();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		delete my_serv;
		return (1);
	}

	delete my_serv;	
	return (0);
}
