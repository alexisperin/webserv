#!/usr/bin/env perl

=head1 DESCRIPTION

printenv — a CGI program that just prints its environment
to use: perl resources/cgi/printenv.pl

=cut
print "HTTP/1.1 200 OK\nContent-Type: text/plain\n\n";

foreach ( sort keys %ENV ) {
    print "$_=\"$ENV{$_}\"\n";
}