/* Jason Boutte
 * Tyler Parks
 *
 * CSCI 446
 * Fall 2014
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap/pcap.h>
#include <arpa/inet.h>
#include <netinet/ether.h>

#define TIMEOUT_MS 30000

// https://en.wikipedia.org/wiki/EtherType 
#define IPv4 0x0800 // Ethertype for IPv4
#define IPv6 0x86DD // Ethertype for IPv6

// https://en.wikipedia.org/wiki/List_of_IP_protocol_numbers
#define TCP 0x06 // TCP packet
#define UDP 0x11 // UPD packet

#define msg(M, ...) fprintf(stdout, M, ##__VA_ARGS__)
#define error(M, ...) fprintf(stderr, M "\n", ##__VA_ARGS__)

// Helper function pushes 16 bits onto
// a 32 bit data type
int read_2bytes(const u_char *buf);

int main(int argc, char **argv) {
	int use_file = 0;
	pcap_t *handle; // pcap handle used to read packets
	char err_buf[PCAP_ERRBUF_SIZE]; // pcap error buffer

	// Check for correct number of arguments
	if (argc > 2) {
		msg("Usage: %s file\n", argv[0]);

		exit(0);
	}

	// Open pcap handle based on provided arguments
	if (argc > 1) {
		use_file = 1;

		// Attempts to open file provided as first argument
		handle = pcap_open_offline(argv[1], err_buf);

		if (handle == NULL) {
			error("Open file error:\n%s\n", err_buf);	
	
			exit(0);
		}		

		msg("Processing file '%s'\n", argv[1]);
	} else {
		char *dev = NULL;

		use_file = 0;

		// Gets the default device
		dev = pcap_lookupdev(err_buf);

		if (dev == NULL) {
			error("Device lookup error:\n%s\n", err_buf);

			exit(0);
		}

		// Open pcap handle to start capturing packets from interface	
		handle = pcap_open_live(dev, BUFSIZ, 1, TIMEOUT_MS, err_buf);

		if (handle == NULL) {
			error("Device open error:\n%s\n", err_buf);		

			exit(0);
		}

		msg("Capturing on interface '%s'\n", dev);
	}	

	int ret;
	int port; // Stores port value
	char *mac; // Used to store mac address in string form
	int ip_proto; // Holds ip protocol id
	int ether_type; // Holds ether type id
	struct in_addr addr; // IPv4 structure
	struct in6_addr addr6; // IPv6 structure
	struct ether_addr eth; // Ethernet structure
	struct pcap_pkthdr *hdr; // Pcap packet structure
	char ip_buf[INET6_ADDRSTRLEN]; // Buffer for converting 4/16 byte address
	const u_char *pkt, *pkt_index; // Data buffer and index

	// Iterate through ethernet packets
	while (1) {
		ret = pcap_next_ex(handle, &hdr, &pkt);

		// Check for errors
		if (ret == 0 && !use_file) {
			error("Live capture timeout expired\n");
			break;
		} else if (ret == -1) {
			error("Error reading packet\n");
			break;
		} else if (ret == -2) {
			break;
		}

		// Sets pointer to beginning of packet data
		pkt_index	= pkt;

		// Source ethernet address is stored after the 
		// destination, copy from after the destinations
		// 6 bytes
		memcpy(eth.ether_addr_octet, pkt_index+6, 6);

		// Translate 4 bytes to xx.xx.xx.xx.xx.xx. format
		// On failure skip to next packet
		if ((mac = ether_ntoa(&eth)) == NULL) {
			error("Error translating ethernet address\n");

			continue;
		}

		msg("%s -> ", mac);

		// Copy destination ethernet address
		memcpy(eth.ether_addr_octet, pkt_index, 6);

		// Step index past destination and source bytes
		pkt_index += 12;

		// Translate 4 bytes to xx.xx.xx.xx.xx.xx. format
		// On failure skip to next packet
		if ((mac = ether_ntoa(&eth)) == NULL) {
			error("\nError translating ethernet address\n");

			continue;
		}

		msg("%s\n", mac);

		// Get ethernet frame type
		ether_type = read_2bytes(pkt_index);

		pkt_index += 2;

		// Process different ethernet frame types
		if (ether_type == IPv4) {
			msg("\t[IPv4] ");

			// Skip 9 bytes ahead
			pkt_index += 9;

			// Get contained IP protocol
			ip_proto = *pkt_index;

			pkt_index += 3;

			// Copy source address to in_addr structure
			memcpy(&addr.s_addr, pkt_index, 4);

			pkt_index += 4;

			// Translate 4 byte address to x.x.x.x format
			if (inet_ntop(AF_INET, &addr, ip_buf, INET_ADDRSTRLEN) == NULL) {
				error("\nError translating IPv4 address\n");

				continue;
			}		
			
			msg("%s -> ", ip_buf);	

			// Copy destination address to in_addr structure
			memcpy(&addr.s_addr, pkt_index, 4);

			pkt_index += 4;

			// Traslate 4 byte address to x.x.x.x format
			if (inet_ntop(AF_INET, &addr, ip_buf, INET_ADDRSTRLEN) == NULL) {
				error("\nError translating IPv4 address\n");

				continue;
			}

			msg("%s\n", ip_buf);
		} else if (ether_type == IPv6) {
			msg("\t[IPv6] ");	

			pkt_index += 6;

			// Get contained IP protocol
			ip_proto = *pkt_index;

			pkt_index += 2;

			// Copy 16 bytes to in6_addr structure
			memcpy(&addr6.s6_addr, pkt_index, 16);

			pkt_index += 16;

			// Translate 16 byte address to dot notation
			if (inet_ntop(AF_INET6, &addr6, ip_buf, INET6_ADDRSTRLEN) == NULL) {
				error("\nError translating IPv6 address\n");

				continue;
			}

			msg("%s -> ", ip_buf);			

			// Copy 16 bytes to in6_addr structure
			memcpy(&addr6.s6_addr, pkt_index, 16);

			pkt_index += 16;

			// Translate 16 byte address to dot notation
			if (inet_ntop(AF_INET6, &addr6, ip_buf, INET6_ADDRSTRLEN) == NULL) {
				error("\nError translating IPv6 address\n");

				continue;
			}

			msg("%s\n", ip_buf);
		} else {
			// Print unknown ethernet type
			msg("\t[%d]\n", ether_type);

			continue;
		}

		// Process IP protocols, we'll only handle TCP and UDP
		// and we only care about the port numbers which are
		// in the same location in the bitstream so we don't
		// need to handle them separately
		if (ip_proto == TCP || ip_proto == UDP) {
			msg("\t[%s] ", (ip_proto == TCP) ? "TCP" : "UDP");

			// Read first 2 bytes for source port
			port = read_2bytes(pkt_index);

			pkt_index += 2;

			msg("%d -> ", port);

			// Read second 2 bytes for destination port
			port = read_2bytes(pkt_index);

			pkt_index += 2;

			msg("%d\n", port);			
		} else {
			msg("\t[%d]\n", ip_proto);

			continue;
		}
	}

	// Close pcap handle
	pcap_close(handle);

  return 0;
}

int read_2bytes(const u_char *buf) {
	int x;
	int ret = 0;

	for (x = 0; x < 2; ++x) {
		ret |= *(buf++) << ((1-x)*8);
	}	

	return ret;
}
