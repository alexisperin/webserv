/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/25 13:10:31 by yhuberla          #+#    #+#             */
/*   Updated: 2023/05/25 18:16:42 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "Webserv.hpp"
# include <algorithm>

class Location
{
	private:
		bool _auto_index;
		bool _auto_sighted;
		bool _line_sighted;
		bool _return_sighted;

		void compare_block_info(std::string line);
		void check_set_default(void);

	public:
		Location(std::string line, std::ifstream & indata, std::string root);
		~Location(void);
		Location &operator=(const Location & other);

		std::string _location;
		std::string _root;
		std::list<std::string> _index_files;
		std::vector<std::string> _methods;
		std::string _return;

		void display_loc_content(void);
};

#endif
