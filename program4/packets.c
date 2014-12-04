#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ether.h>
#include <pcap/pcap.h>

#define IPv4 0x0800
#define IPv6 0x86DD

#define debug(M, ...) fprintf(stderr, "%s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define BUF_SIZE 1024

void peek(const u_char *s, int len);

int main(int argc, char **argv) {
  char *dev = NULL;
  char buf[BUF_SIZE];
  char use_file = 0;
  char *trace_file = NULL;
  pcap_t *handle;
  struct pcap_pkthdr *header;
  const u_char *packet;
  int ret;

  if (argc > 2) {
    fprintf(stderr, "Usage: %s file\n", argv[0]);

    return 1;
  } else if (argc > 1) {
    use_file = 1;
    
    trace_file = argv[1];
  } else {
    use_file = 0;
  }

  if (use_file) {
    handle = pcap_open_offline(trace_file, buf);

    if (handle == NULL) {
      fprintf(stderr, "Error opening file %s\n", trace_file);

      return 1;
    }
  } else {
    dev = pcap_lookupdev(buf);

    if (dev == NULL) {
      fprintf(stderr, "Couldn't find device %s\n", buf);

      return 1;
    }

    handle = pcap_open_live(dev, BUF_SIZE, 1, 1000, buf);

    if (handle == NULL) {
      fprintf(stderr, "Couldn't open device %s\n", dev);

      return 1;
    }
  }

  int x;
  int type;
  struct ether_addr eth;
  char *mac = NULL;
  int index = 0;

  do {
    ret = pcap_next_ex(handle, &header, &packet);

    if (ret < 0) {
      fprintf(stderr, "Error get packet\n");

      pcap_close(handle);

      return 1;
    }

    memcpy(eth.ether_addr_octet, packet, 6);
  
    index += 6;

    mac = ether_ntoa(&eth);

    fprintf(stdout, "%s -> ", mac); 

    memcpy(eth.ether_addr_octet, packet+index, 6);

    index += 6;

    mac = ether_ntoa(&eth);

    fprintf(stdout, "%s\n", mac);

    type = *(packet+index+1) << 8;

    type |= *(packet+index+2);

    index += 2;

    fprintf(stdout, "0x%.2x\n", type);

    if (type == IPv4) {

    } else if (type == IPv6) {

    } else {

    }
  } while (1);

  pcap_close(handle);

  return 0;
}

void peek(const u_char *s, int len) {
  int x;

  for (x = 0; x < len; ++x) {
    fprintf(stdout, "%.2x\t", s[x]);
  }

  fprintf(stdout, "\n");
}
