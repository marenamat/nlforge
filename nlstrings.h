#define STR(x) [x] = #x
#define FLSTR(x) [__builtin_ffs(x)-1] = #x

const char * const nlmsg_type_string[] = {
  STR(NLMSG_NOOP),
  STR(NLMSG_ERROR),
  STR(NLMSG_DONE),
  STR(NLMSG_OVERRUN),
  STR(RTM_NEWLINK),
  STR(RTM_DELLINK),
  STR(RTM_GETLINK),
  STR(RTM_SETLINK),
  STR(RTM_NEWADDR),
  STR(RTM_DELADDR),
  STR(RTM_GETADDR),
  STR(RTM_NEWROUTE),
  STR(RTM_DELROUTE),
  STR(RTM_GETROUTE),
  STR(RTM_NEWNEIGH),
  STR(RTM_DELNEIGH),
  STR(RTM_GETNEIGH),
  STR(RTM_NEWRULE),
  STR(RTM_DELRULE),
  STR(RTM_GETRULE),
  STR(RTM_NEWQDISC),
  STR(RTM_DELQDISC),
  STR(RTM_GETQDISC),
  STR(RTM_NEWTCLASS),
  STR(RTM_DELTCLASS),
  STR(RTM_GETTCLASS),
  STR(RTM_NEWTFILTER),
  STR(RTM_DELTFILTER),
  STR(RTM_GETTFILTER),
  STR(RTM_NEWACTION),
  STR(RTM_DELACTION),
  STR(RTM_GETACTION),
  STR(RTM_NEWPREFIX),
  STR(RTM_GETMULTICAST),
  STR(RTM_GETANYCAST),
  STR(RTM_NEWNEIGHTBL),
  STR(RTM_GETNEIGHTBL),
  STR(RTM_SETNEIGHTBL),
  STR(RTM_NEWNDUSEROPT),
  STR(RTM_NEWADDRLABEL),
  STR(RTM_DELADDRLABEL),
  STR(RTM_GETADDRLABEL),
  STR(RTM_GETDCB),
  STR(RTM_SETDCB),
  STR(RTM_NEWNETCONF),
  STR(RTM_GETNETCONF),
  STR(RTM_NEWMDB),
  STR(RTM_DELMDB),
  STR(RTM_GETMDB),
  STR(RTM_NEWNSID),
  STR(RTM_DELNSID),
  STR(RTM_GETNSID),
};

const char * const af_string[] = {
  STR(AF_UNSPEC),
  STR(AF_LOCAL),
  STR(AF_INET),
  STR(AF_AX25),
  STR(AF_IPX),
  STR(AF_APPLETALK),
  STR(AF_NETROM),
  STR(AF_BRIDGE),
  STR(AF_ATMPVC),
  STR(AF_X25),
  STR(AF_INET6),
  STR(AF_ROSE),
  STR(AF_DECnet),
  STR(AF_NETBEUI),
  STR(AF_SECURITY),
  STR(AF_KEY),
  STR(AF_NETLINK),
  STR(AF_PACKET),
  STR(AF_ASH),
  STR(AF_ECONET),
  STR(AF_ATMSVC),
  STR(AF_RDS),
  STR(AF_SNA),
  STR(AF_IRDA),
  STR(AF_PPPOX),
  STR(AF_WANPIPE),
  STR(AF_LLC),
  STR(AF_IB),
  STR(AF_MPLS),
  STR(AF_CAN),
  STR(AF_TIPC),
  STR(AF_BLUETOOTH),
  STR(AF_IUCV),
  STR(AF_RXRPC),
  STR(AF_ISDN),
  STR(AF_PHONET),
  STR(AF_IEEE802154),
  STR(AF_CAIF),
  STR(AF_ALG),
  STR(AF_NFC),
  STR(AF_VSOCK),
  STR(AF_MAX),
};

#define rtm_family_string af_string

const char * const rtm_protocol_string[] = {
  STR(RTPROT_UNSPEC),
  STR(RTPROT_REDIRECT),
  STR(RTPROT_KERNEL),
  STR(RTPROT_BOOT),
  STR(RTPROT_STATIC),
  STR(RTPROT_GATED),
  STR(RTPROT_RA),
  STR(RTPROT_MRT),
  STR(RTPROT_ZEBRA),
  STR(RTPROT_BIRD),
  STR(RTPROT_DNROUTED),
  STR(RTPROT_XORP),
  STR(RTPROT_NTK),
  STR(RTPROT_DHCP),
  STR(RTPROT_MROUTED),
  STR(RTPROT_BABEL),
};

const char * const rtm_scope_string[] = {
  STR(RT_SCOPE_UNIVERSE),
  STR(RT_SCOPE_SITE),
  STR(RT_SCOPE_LINK),
  STR(RT_SCOPE_HOST),
  STR(RT_SCOPE_NOWHERE),
};

const char * const rtm_type_string[] = {
  STR(RTN_UNSPEC),
  STR(RTN_UNICAST),
  STR(RTN_LOCAL),
  STR(RTN_BROADCAST),
  STR(RTN_ANYCAST),
  STR(RTN_MULTICAST),
  STR(RTN_BLACKHOLE),
  STR(RTN_UNREACHABLE),
  STR(RTN_PROHIBIT),
  STR(RTN_THROW),
  STR(RTN_NAT),
  STR(RTN_XRESOLVE),
};

const char * const rtm_flags_string[] = {
  FLSTR(RTM_F_NOTIFY),
  FLSTR(RTM_F_CLONED),
  FLSTR(RTM_F_EQUALIZE),
  FLSTR(RTM_F_PREFIX),
  FLSTR(RTM_F_LOOKUP_TABLE),
};

const char * const rta_type_string[] = {
  STR(RTA_UNSPEC),
  STR(RTA_DST),
  STR(RTA_SRC),
  STR(RTA_IIF),
  STR(RTA_OIF),
  STR(RTA_GATEWAY),
  STR(RTA_PRIORITY),
  STR(RTA_PREFSRC),
  STR(RTA_METRICS),
  STR(RTA_MULTIPATH),
  STR(RTA_PROTOINFO),
  STR(RTA_FLOW),
  STR(RTA_CACHEINFO),
  STR(RTA_SESSION),
  STR(RTA_MP_ALGO),
  STR(RTA_TABLE),
  STR(RTA_MARK),
  STR(RTA_MFC_STATS),
  STR(RTA_VIA),
  STR(RTA_NEWDST),
  STR(RTA_PREF),
  STR(RTA_ENCAP_TYPE),
  STR(RTA_ENCAP),
  STR(RTA_EXPIRES),
};

const char * const rtnh_flags_string[] = {
  FLSTR(RTNH_F_DEAD),
  FLSTR(RTNH_F_PERVASIVE),
  FLSTR(RTNH_F_ONLINK),
  FLSTR(RTNH_F_OFFLOAD),
  FLSTR(RTNH_F_LINKDOWN),
};

#define rtvia_family_string af_string

#define NLMSG_FLAGS_STRING_BASIC FLSTR(NLM_F_REQUEST), FLSTR(NLM_F_MULTI), FLSTR(NLM_F_ACK), FLSTR(NLM_F_ECHO), FLSTR(NLM_F_DUMP_INTR), FLSTR(NLM_F_DUMP_FILTERED)
#define NLMSG_FLAGS_STRING_GET FLSTR(NLM_F_ROOT), FLSTR(NLM_F_MATCH), FLSTR(NLM_F_ATOMIC)
#define NLMSG_FLAGS_STRING_NEW FLSTR(NLM_F_REPLACE), FLSTR(NLM_F_EXCL), FLSTR(NLM_F_CREATE), FLSTR(NLM_F_APPEND)

const char * const nlmsg_flags_string_basic[] = {
  NLMSG_FLAGS_STRING_BASIC
};

const char * const nlmsg_flags_string_new[] = {
  NLMSG_FLAGS_STRING_BASIC, NLMSG_FLAGS_STRING_NEW
};

const char * const nlmsg_flags_string_get[] = {
  NLMSG_FLAGS_STRING_BASIC, NLMSG_FLAGS_STRING_GET
};

#define MAXFLAGLEN  65536

#define strflags(flags, def) _strflags(flags, def, sizeof(def)/sizeof(*def))

static inline const char const *_strflags(u32 flags, const char * const def[], int deflen) {
  static char out[MAXFLAGLEN];
  int pos = 0;
  for (int i=0; flags >= (1UL << i); i++)
    if (flags & (1UL << i)) {
      if (pos) {
	strcpy(out + pos, " | ");
	pos += 3;
      }
      if (i < deflen && def[i]) {
	strcpy(out + pos, def[i]);
	pos += strlen(def[i]);
      } else {
	strcpy(out + pos, "(unknown)");
	pos += strlen("(unknown)");
      }
    }

  if (pos == 0)
    return "(none)";

  out[pos] = 0;
  return out;
}
