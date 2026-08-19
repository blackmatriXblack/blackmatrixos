#ifndef SHELL_H
#define SHELL_H

#include "types.h"

#define SHELL_MAX_CMD   512
#define SHELL_MAX_ARGS  64
#define SHELL_HISTORY   32

typedef struct {
    char cmd[SHELL_MAX_CMD];
} shell_history_entry_t;

typedef int (*builtin_cmd_t)(int argc, char** argv);

typedef struct {
    const char*  name;
    builtin_cmd_t handler;
    const char*  help;
} shell_command_t;

void shell_init(void);
void shell_run(void);
void shell_execute(const char* cmdline);
void env_init(void);
const char* env_get(const char* name);
int  env_set(const char* name, const char* value);
int  env_unset(const char* name);
void env_list(void);

int cmd_help(int,char**);    int cmd_clear(int,char**);
int cmd_echo(int,char**);    int cmd_ls(int,char**);
int cmd_cd(int,char**);      int cmd_pwd(int,char**);
int cmd_cat(int,char**);     int cmd_mkdir(int,char**);
int cmd_rmdir(int,char**);   int cmd_rm(int,char**);
int cmd_touch(int,char**);   int cmd_cp(int,char**);
int cmd_mv(int,char**);      int cmd_rename(int,char**);
int cmd_chmod(int,char**);   int cmd_chown(int,char**);
int cmd_ln(int,char**);      int cmd_hardlink(int,char**);
int cmd_find(int,char**);    int cmd_tree(int,char**);
int cmd_which(int,char**);   int cmd_whereis(int,char**);
int cmd_locate(int,char**);  int cmd_stat(int,char**);
int cmd_df(int,char**);      int cmd_du(int,char**);
int cmd_file(int,char**);    int cmd_readlink(int,char**);
int cmd_mkfifo(int,char**);  int cmd_mknod(int,char**);
int cmd_mount(int,char**);   int cmd_umount(int,char**);
int cmd_swapon(int,char**);  int cmd_swapoff(int,char**);
int cmd_mkfs(int,char**);    int cmd_fsck(int,char**);
int cmd_dd(int,char**);      int cmd_xargs(int,char**);
int cmd_install(int,char**); int cmd_truncate(int,char**);
int cmd_mktemp(int,char**);  int cmd_realpath(int,char**);
int cmd_sync(int,char**);
int cmd_grep(int,char**);    int cmd_wc(int,char**);
int cmd_head(int,char**);    int cmd_tail(int,char**);
int cmd_sort(int,char**);    int cmd_uniq(int,char**);
int cmd_rev(int,char**);     int cmd_tac(int,char**);
int cmd_nl(int,char**);      int cmd_cut(int,char**);
int cmd_paste(int,char**);   int cmd_fold(int,char**);
int cmd_expand(int,char**);  int cmd_unexpand(int,char**);
int cmd_tr(int,char**);      int cmd_comm(int,char**);
int cmd_diff(int,char**);    int cmd_patch(int,char**);
int cmd_cmp(int,char**);     int cmd_sdiff(int,char**);
int cmd_tee(int,char**);     int cmd_shuf(int,char**);
int cmd_column(int,char**);  int cmd_pr(int,char**);
int cmd_fmt(int,char**);     int cmd_indent(int,char**);
int cmd_tabs(int,char**);    int cmd_colrm(int,char**);
int cmd_col(int,char**);     int cmd_join(int,char**);
int cmd_split(int,char**);   int cmd_csplit(int,char**);
int cmd_od(int,char**);      int cmd_hexdump(int,char**);
int cmd_strings(int,char**); int cmd_look(int,char**);
int cmd_pell(int,char**);    int cmd_spell(int,char**);
int cmd_more(int,char**);    int cmd_envsubst(int,char**);
int cmd_base64(int,char**);  int cmd_sum(int,char**);
int cmd_cksum(int,char**);   int cmd_md5sum(int,char**);
int cmd_sha1sum(int,char**); int cmd_sha256sum(int,char**);
int cmd_ps(int,char**);      int cmd_kill(int,char**);
int cmd_killall(int,char**); int cmd_pgrep(int,char**);
int cmd_top(int,char**);     int cmd_jobs(int,char**);
int cmd_bg(int,char**);      int cmd_fg(int,char**);
int cmd_nice(int,char**);    int cmd_renice(int,char**);
int cmd_nohup(int,char**);   int cmd_wait(int,char**);
int cmd_timeout(int,char**); int cmd_watch(int,char**);
int cmd_at(int,char**);      int cmd_cron(int,char**);
int cmd_crontab(int,char**); int cmd_sleep(int,char**);
int cmd_timecmd(int,char**);
int cmd_mem(int,char**);     int cmd_uptime(int,char**);
int cmd_ver(int,char**);     int cmd_uname(int,char**);
int cmd_date(int,char**);    int cmd_time(int,char**);
int cmd_cal(int,char**);     int cmd_hostname(int,char**);
int cmd_whoami(int,char**);  int cmd_id(int,char**);
int cmd_lscpu(int,char**);   int cmd_lsblk(int,char**);
int cmd_lsusb(int,char**);   int cmd_lspci(int,char**);
int cmd_lsof(int,char**);    int cmd_dmesg(int,char**);
int cmd_free(int,char**);    int cmd_systeminfo(int,char**);
int cmd_locale(int,char**);  int cmd_getconf(int,char**);
int cmd_nproc(int,char**);   int cmd_arch(int,char**);
int cmd_tty(int,char**);     int cmd_stty(int,char**);
int cmd_tput(int,char**);    int cmd_groups(int,char**);
int cmd_logname(int,char**); int cmd_last(int,char**);
int cmd_w(int,char**);       int cmd_users(int,char**);
int cmd_finger(int,char**);  int cmd_neofetch(int,char**);
int cmd_env(int,char**);     int cmd_export(int,char**);
int cmd_printenv(int,char**);int cmd_setenv(int,char**);
int cmd_alias(int,char**);   int cmd_unalias(int,char**);
int cmd_set(int,char**);     int cmd_unset(int,char**);
int cmd_read(int,char**);    int cmd_declare(int,char**);
int cmd_local(int,char**);   int cmd_getopts(int,char**);
int cmd_true(int,char**);    int cmd_false(int,char**);
int cmd_yes(int,char**);     int cmd_history(int,char**);
int cmd_test(int,char**);    int cmd_test_bracket(int,char**);
int cmd_printf(int,char**);  int cmd_source(int,char**);
int cmd_exit(int,char**);    int cmd_break(int,char**);
int cmd_continue(int,char**);int cmd_return(int,char**);
int cmd_trap(int,char**);    int cmd_exec(int,char**);
int cmd_eval(int,char**);    int cmd_shift(int,char**);
int cmd_let(int,char**);
int cmd_calc(int,char**);    int cmd_expr(int,char**);
int cmd_factor(int,char**);  int cmd_seq(int,char**);
int cmd_hex(int,char**);     int cmd_dec(int,char**);
int cmd_bitcalc(int,char**); int cmd_units(int,char**);
int cmd_numfmt(int,char**);
int cmd_ifconfig(int,char**);int cmd_ip(int,char**);
int cmd_netstat(int,char**); int cmd_ping(int,char**);
int cmd_traceroute(int,char**);int cmd_nslookup(int,char**);
int cmd_dig(int,char**);     int cmd_host(int,char**);
int cmd_wget(int,char**);    int cmd_curl(int,char**);
int cmd_netsh(int,char**);   int cmd_route(int,char**);
int cmd_arp(int,char**);     int cmd_ss(int,char**);
int cmd_nc(int,char**);      int cmd_telnet(int,char**);
int cmd_ftp(int,char**);     int cmd_ssh(int,char**);
int cmd_scp(int,char**);     int cmd_sftp(int,char**);
int cmd_nmap(int,char**);    int cmd_whois(int,char**);
int cmd_gzip(int,char**);    int cmd_gunzip(int,char**);
int cmd_zip(int,char**);     int cmd_unzip(int,char**);
int cmd_tar(int,char**);     int cmd_compress(int,char**);
int cmd_bzip2(int,char**);   int cmd_xz(int,char**);
int cmd_wmic(int,char**);    int cmd_sfc(int,char**);
int cmd_chkdsk(int,char**);  int cmd_diskpart(int,char**);
int cmd_reg(int,char**);     int cmd_sc(int,char**);
int cmd_net(int,char**);     int cmd_powershell(int,char**);
int cmd_cmd(int,char**);     int cmd_driverquery(int,char**);
int cmd_gpresult(int,char**);int cmd_shutdown(int,char**);
int cmd_logoff(int,char**);  int cmd_assoc(int,char**);
int cmd_ftype(int,char**);   int cmd_title(int,char**);
int cmd_prompt(int,char**);  int cmd_doskey(int,char**);
int cmd_mode(int,char**);
int cmd_strace(int,char**);  int cmd_ltrace(int,char**);
int cmd_ldd(int,char**);     int cmd_objdump(int,char**);
int cmd_nm(int,char**);      int cmd_size(int,char**);
int cmd_strip(int,char**);   int cmd_readelf(int,char**);
int cmd_cowsay(int,char**);  int cmd_cowthink(int,char**);
int cmd_matrix(int,char**);  int cmd_gui(int,char**);
int cmd_color(int,char**);   int cmd_sl(int,char**);
int cmd_fortune(int,char**); int cmd_figlet(int,char**);
int cmd_banner(int,char**);  int cmd_toilet(int,char**);
int cmd_rot13(int,char**);   int cmd_morse(int,char**);
int cmd_pi(int,char**);      int cmd_random(int,char**);
int cmd_dice(int,char**);    int cmd_coin(int,char**);
int cmd_rps(int,char**);     int cmd_quiz(int,char**);
int cmd_basename(int,char**);int cmd_dirname(int,char**);
int cmd_reboot(int,char**);  int cmd_halt(int,char**);
int cmd_lspci(int,char**);   int cmd_netstat(int,char**);

/* Scripting engine */
void shell_handle_line(const char* line);
void shell_script_file(const char* path);

/* Editor */
int cmd_edit(int,char**);

/* Games */
int cmd_game_2048(int,char**);   int cmd_snake(int,char**);
int cmd_tictactoe(int,char**);   int cmd_hangman(int,char**);

/* Math (fixed-point) */
int cmd_sci(int,char**);         int cmd_stats(int,char**);
int cmd_primes(int,char**);      int cmd_fib(int,char**);
int cmd_roman(int,char**);       int cmd_angles(int,char**);

/* Ciphers & encoding */
int cmd_caesar(int,char**);      int cmd_vigenere(int,char**);
int cmd_atbash(int,char**);      int cmd_urlencode(int,char**);
int cmd_urldecode(int,char**);   int cmd_hash(int,char**);

/* Animations & fun */
int cmd_donut(int,char**);       int cmd_stars(int,char**);
int cmd_clock(int,char**);       int cmd_hollywood(int,char**);
int cmd_progress(int,char**);

/* System */
int cmd_bench(int,char**);       int cmd_at(int,char**);

#endif
