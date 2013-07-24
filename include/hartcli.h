#ifndef _INCLUDED_HARTCLI_H
#define _INCLUDED_HARTCLI_H
void handle_set_position(struct hip_sess *sess, hip_s16 pos);
void handle_clear_alarms(struct hip_sess *sess);
void handle_read(struct hip_sess *sess);
void handle_enum(struct hip_sess *sess);
void handle_get_long_tag(struct hip_sess *sess);
void handle_set_long_tag(struct hip_sess *sess);
void handle_get_nickname(struct hip_sess *sess);
void handle_get_netid(struct hip_sess *sess);
void handle_get_network_stat(struct hip_sess *sess);
void handle_get_neighbor_list(struct hip_sess *sess);
void handle_get_neighbor_dlist(struct hip_sess *sess);
void handle_get_route_list(struct hip_sess *sess);

#endif
