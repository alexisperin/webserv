// #!py
// import sys

// def run(socket_fd, body):
// 	print(socket_fd + " with a body of " + body)

// if __name__ == "__main__":
// 	ac = len(sys.argv)
// 	if ac == 3:
// 		run(sys.argv[1], sys.argv[2])
// 	else:
// 		print("Hello world!")

#include <iostream>
#include <sys/socket.h>

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Usage: ./cgi_script socket_fd body" << std::endl;
		std::cerr << "Got instead: ./" << av[0] << ' ';
		for (int index = 1; index < ac; index++)
			std::cerr << av[index] << ' ';
		std::cerr << std::endl;
		return (1);
	}
	std::cerr << av[1] << " with a body of " << av[2] << std::endl;
	std::string content = "HTTP/1.1 200 OK\n\n";
	content += "<HTML>\n	<HEAD>\n		<TITLE>Your Title Here</TITLE>\n	</HEAD>\n	<BODY BGCOLOR=\"FFFFFF\">\n		<img src=\"images/chesshome.png\" alt=\"Chess Home\">\n	</BODY>\n</HTML>";
	send(atoi(av[1]), content.c_str(), content.size(), 0);
	return (0);
}