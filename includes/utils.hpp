/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/25 13:38:43 by yhuberla          #+#    #+#             */
/*   Updated: 2023/05/31 17:35:24 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <fstream>
# include <iostream>
# include <stdlib.h>
# include <dirent.h>
# include "Webserv.hpp"

std::string	trim_spaces(std::string str);
std::string read_data(std::ifstream &indata);
int is_error_code(int code);
int check_http_version(std::string bufstr);
int check_correct_host(std::string bufstr);
int check_header_names(std::string bufstr);
void display_special_characters(std::string str);
void run_script(int socket_fd, std::string body);
std::string get_body(std::string bufstr);
char	*ft_itoa(int n);
std::string get_first_index_file(std::string root, std::string prev_loc, std::list<std::string> index_files, bool auto_index);

#endif
