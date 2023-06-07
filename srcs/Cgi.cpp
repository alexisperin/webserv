/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/05 15:37:51 by yhuberla          #+#    #+#             */
/*   Updated: 2023/06/05 15:47:55 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"

Cgi::Cgi(std::string header, std::string file_path, Server *serv)
{
	(void)header;
	(void)file_path;
	(void)serv;
	std::cout << "Constructor of cgi called" << std::endl;

	//setup env

	//get body if needed with some more recv()

	//send said body to std::cout after duping it

	//fork and call cgi

	//read from std::in to send message back to server

	//parsing of read input

	// throw Webserv::QuickerReturnException(); 
}

Cgi::~Cgi(void)
{
}

// ************************************************************************** //
//                                  Private                                   //
// ************************************************************************** //

// ************************************************************************** //
//                                  Public                                    //
// ************************************************************************** //
