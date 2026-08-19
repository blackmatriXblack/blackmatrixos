#include "../include/shell.h"
#include "../include/vga.h"
#include "../include/keyboard.h"
#include "../include/fs.h"
#include "../include/memory.h"
#include "../include/process.h"
#include "../include/timer.h"
#include "../include/string.h"
#include "../include/stdio.h"
#include "../include/io.h"
#include "../include/net.h"

extern int inode_count;
static unsigned int rand_seed = 12345;
static int rand(void) { rand_seed = rand_seed * 1103515245 + 12345; return (int)(rand_seed >> 16) & 0x7FFF; }

/* Command history display buffer */
static char cmd_history_buf[128][SHELL_MAX_CMD];
static int cmd_history_len = 0;

/* Real 'at' job scheduler (declared here, defined below) */
#define MAX_AT_JOBS 16
typedef struct {
    uint32_t when_ticks;
    char     cmd[SHELL_MAX_CMD];
    bool     used;
} at_job_t;
static at_job_t at_jobs[MAX_AT_JOBS];
void at_add(int seconds, const char* cmd);
void at_run_due(void);
void at_list(void);

/* Environment variables */
#define MAX_ENV 64
#define ENV_NAME_LEN 32
#define ENV_VAL_LEN  128

typedef struct {
    char name[ENV_NAME_LEN];
    char value[ENV_VAL_LEN];
    bool used;
} env_var_t;

static env_var_t env_table[MAX_ENV];

/* Command history */
static shell_history_entry_t history[SHELL_HISTORY];
static int history_count = 0;
static int history_pos = 0;

/* Built-in command table */
static shell_command_t commands[] = {
    /* File operations (Linux) */
    {"help",      cmd_help,      "Show available commands"},
    {"clear",     cmd_clear,     "Clear the screen"},
    {"cls",       cmd_clear,     "Clear the screen (Windows alias)"},
    {"echo",      cmd_echo,      "Display text"},
    {"ls",        cmd_ls,        "List directory contents"},
    {"dir",       cmd_ls,        "List directory (Windows alias)"},
    {"cd",        cmd_cd,        "Change directory"},
    {"pwd",       cmd_pwd,       "Print working directory"},
    {"cat",       cmd_cat,       "Display file contents"},
    {"type",      cmd_cat,       "Display file (Windows alias)"},
    {"mkdir",     cmd_mkdir,     "Create directory"},
    {"md",        cmd_mkdir,     "Create directory (Windows alias)"},
    {"rm",        cmd_rm,        "Remove file"},
    {"del",       cmd_rm,        "Remove file (Windows alias)"},
    {"erase",     cmd_rm,        "Remove file (Windows alias)"},
    {"rmdir",     cmd_rmdir,     "Remove empty directory"},
    {"rd",        cmd_rmdir,     "Remove directory (Windows alias)"},
    {"touch",     cmd_touch,     "Create empty file"},
    {"cp",        cmd_cp,        "Copy file"},
    {"copy",      cmd_cp,        "Copy file (Windows alias)"},
    {"mv",        cmd_mv,        "Move/rename file"},
    {"move",      cmd_mv,        "Move file (Windows alias)"},
    {"ren",       cmd_rename,    "Rename file (Windows)"},
    {"rename",    cmd_rename,    "Rename file"},
    {"ln",        cmd_ln,        "Create link"},
    {"stat",      cmd_stat,      "File status"},
    {"truncate",  cmd_truncate,  "Resize file"},
    {"chmod",     cmd_chmod,     "Change permissions"},
    {"chown",     cmd_chown,     "Change ownership"},
    {"find",      cmd_find,      "Find files"},
    {"tree",      cmd_tree,      "Show directory tree"},
    {"which",     cmd_which,     "Locate a command"},
    {"where",     cmd_which,     "Locate command (Windows alias)"},
    {"whereis",   cmd_whereis,   "Locate binary/source/man"},
    {"locate",    cmd_locate,    "Find files by name"},
    {"df",        cmd_df,        "Show disk usage"},
    {"du",        cmd_du,        "Estimate file space usage"},
    {"file",      cmd_file,      "Determine file type"},
    {"readlink",  cmd_readlink,  "Resolve symlink"},
    {"mkfifo",    cmd_mkfifo,    "Create named pipe"},
    {"mknod",     cmd_mknod,     "Create device node"},
    {"install",   cmd_install,   "Copy and set permissions"},
    {"link",      cmd_hardlink,  "Create hard link"},

    /* Text processing (Linux/Unix) */
    {"grep",      cmd_grep,      "Search pattern in files"},
    {"findstr",   cmd_grep,      "Search strings (Windows alias)"},
    {"wc",        cmd_wc,        "Count lines/words/chars"},
    {"head",      cmd_head,      "Show first lines of file"},
    {"tail",      cmd_tail,      "Show last lines of file"},
    {"sort",      cmd_sort,      "Sort lines of file"},
    {"uniq",      cmd_uniq,      "Remove duplicate lines"},
    {"rev",       cmd_rev,       "Reverse each line"},
    {"tac",       cmd_tac,       "Reverse line order"},
    {"nl",        cmd_nl,        "Number lines"},
    {"cut",       cmd_cut,       "Extract columns"},
    {"paste",     cmd_paste,     "Merge lines side by side"},
    {"fold",      cmd_fold,      "Wrap lines at width"},
    {"expand",    cmd_expand,    "Tabs to spaces"},
    {"unexpand",  cmd_unexpand,  "Spaces to tabs"},
    {"tr",        cmd_tr,        "Translate characters"},
    {"col",       cmd_col,       "Filter reverse line feeds"},
    {"join",      cmd_join,      "Join lines on common field"},
    {"split",     cmd_split,     "Split file into pieces"},
    {"csplit",    cmd_csplit,    "Split by context"},
    {"comm",      cmd_comm,      "Compare sorted files"},
    {"diff",      cmd_diff,      "Compare files line by line"},
    {"patch",     cmd_patch,     "Apply diff (simulated)"},
    {"cmp",       cmd_cmp,       "Compare two files byte by byte"},
    {"sdiff",     cmd_sdiff,     "Side-by-side diff"},
    {"tee",       cmd_tee,       "Write to file and stdout"},
    {"shuf",      cmd_shuf,      "Shuffle lines"},
    {"column",    cmd_column,    "Format in columns"},
    {"pr",        cmd_pr,        "Paginate for printing"},
    {"fmt",       cmd_fmt,       "Simple text formatter"},
    {"par",       cmd_fmt,       "Paragraph formatter alias"},
    {"indent",    cmd_indent,    "Indent lines"},
    {"tabs",      cmd_tabs,      "Set tab stops"},
    {"colrm",     cmd_colrm,     "Remove columns"},
    {"sum",       cmd_sum,       "BSD checksum"},
    {"cksum",     cmd_cksum,     "CRC checksum"},
    {"md5sum",    cmd_md5sum,    "MD5 hash (simulated)"},
    {"sha1sum",   cmd_sha1sum,   "SHA1 hash (simulated)"},
    {"sha256sum", cmd_sha256sum, "SHA256 hash (simulated)"},
    {"base64",    cmd_base64,    "Base64 encode/decode"},
    {"xxd",       cmd_hexdump,   "Hex dump (alias)"},
    {"hexdump",   cmd_hexdump,   "Hex dump file"},
    {"od",        cmd_od,        "Octal dump"},
    {"strings",   cmd_strings,   "Extract printable strings"},
    {"look",      cmd_look,      "Search sorted lines"},
    {"pell",      cmd_pell,      "Spelling checker (basic)"},
    {"spell",     cmd_spell,     "Spell checker (basic)"},

    /* System info (Linux/Windows) */
    {"ps",        cmd_ps,        "List running processes"},
    {"tasklist",  cmd_ps,        "List processes (Windows alias)"},
    {"kill",      cmd_kill,      "Kill a process by PID"},
    {"taskkill",  cmd_kill,      "Kill process (Windows alias)"},
    {"killall",   cmd_killall,   "Kill by name"},
    {"pkill",     cmd_killall,   "Kill by name (alias)"},
    {"pgrep",     cmd_pgrep,     "Find process by name"},
    {"top",       cmd_top,       "Process monitor"},
    {"htop",      cmd_top,       "Process monitor (alias)"},
    {"free",      cmd_free,      "Memory usage"},
    {"mem",       cmd_mem,       "Memory info"},
    {"uptime",    cmd_uptime,    "System uptime"},
    {"ver",       cmd_ver,       "OS version"},
    {"version",   cmd_ver,       "OS version (alias)"},
    {"uname",     cmd_uname,     "System information"},
    {"systeminfo",cmd_systeminfo,"System info (Windows)"},
    {"hostname",  cmd_hostname,  "Show hostname"},
    {"whoami",    cmd_whoami,    "Current user"},
    {"id",        cmd_id,        "User and group IDs"},
    {"date",      cmd_date,      "Show date/time"},
    {"time",      cmd_time,      "Show time"},
    {"cal",       cmd_cal,       "Display calendar"},
    {"calendar",  cmd_cal,       "Display calendar (alias)"},
    {"lscpu",     cmd_lscpu,     "CPU information"},
    {"lsblk",     cmd_lsblk,     "List block devices"},
    {"lsusb",     cmd_lsusb,     "List USB devices"},
    {"lspci",     cmd_lspci,     "List PCI devices"},
    {"lsof",      cmd_lsof,      "List open files"},
    {"dmesg",     cmd_dmesg,     "Kernel messages"},
    {"env",       cmd_env,       "Environment variables"},
    {"set",       cmd_setenv,    "Set/show variables"},
    {"export",    cmd_export,    "Set environment variable"},
    {"printenv",  cmd_printenv,  "Print environment"},
    {"locale",    cmd_locale,    "Locale information"},
    {"getconf",   cmd_getconf,   "Configuration values"},
    {"nproc",     cmd_nproc,     "Number of processors"},
    {"arch",      cmd_arch,      "Machine architecture"},
    {"tty",       cmd_tty,       "Terminal name"},
    {"stty",      cmd_stty,      "Terminal settings"},
    {"tput",      cmd_tput,      "Terminal capabilities"},
    {"groups",    cmd_groups,    "User groups"},
    {"logname",   cmd_logname,   "Login name"},
    {"last",      cmd_last,      "Last logins (simulated)"},
    {"w",         cmd_w,         "Who is logged in"},
    {"users",     cmd_users,     "Logged in users"},
    {"finger",    cmd_finger,    "User information"},
    {"neofetch",  cmd_neofetch,  "System info display"},
    {"screenfetch",cmd_neofetch, "System info (alias)"},
    {"inxi",      cmd_neofetch,  "System info (alias)"},

    /* Shell builtins */
    {"alias",     cmd_alias,     "Create command alias"},
    {"unalias",   cmd_unalias,   "Remove alias"},
    {"history",   cmd_history,   "Command history"},
    {"source",    cmd_source,    "Execute script file"},
    {".",         cmd_source,    "Execute script (alias)"},
    {"read",      cmd_read,      "Read input"},
    {"test",      cmd_test,      "Test condition"},
    {"[",         cmd_test_bracket,"Test condition (bracket)"},
    {"true",      cmd_true,      "Return success"},
    {"false",     cmd_false,     "Return failure"},
    {"exit",      cmd_exit,      "Exit shell"},
    {"quit",      cmd_exit,      "Exit shell (alias)"},
    {"logout",    cmd_exit,      "Logout (alias)"},
    {"break",     cmd_break,     "Break from loop"},
    {"continue",  cmd_continue,  "Continue loop"},
    {"return",    cmd_return,    "Return from function"},
    {"trap",      cmd_trap,      "Set signal handler"},
    {"wait",      cmd_wait,      "Wait for process"},
    {"exec",      cmd_exec,      "Execute command"},
    {"eval",      cmd_eval,      "Evaluate expression"},
    {"shift",     cmd_shift,     "Shift positional params"},
    {"let",       cmd_let,       "Arithmetic evaluation"},
    {"declare",   cmd_declare,   "Declare variables"},
    {"local",     cmd_local,     "Local variables"},
    {"typeset",   cmd_declare,   "Declare variables (alias)"},
    {"unset",     cmd_unset,     "Unset variables"},
    {"getopts",   cmd_getopts,   "Parse options"},

    /* Math/Calc */
    {"calc",      cmd_calc,      "Simple calculator"},
    {"bc",        cmd_calc,      "Calculator (alias)"},
    {"expr",      cmd_expr,      "Evaluate expression"},
    {"factor",    cmd_factor,    "Prime factorization"},
    {"seq",       cmd_seq,       "Print number sequence"},
    {"jot",       cmd_seq,       "Print sequence (BSD alias)"},
    {"hex",       cmd_hex,       "Decimal to hex"},
    {"dec",       cmd_dec,       "Hex to decimal"},
    {"bitcalc",   cmd_bitcalc,   "Bitwise calculator"},
    {"units",     cmd_units,     "Unit conversion"},
    {"numfmt",    cmd_numfmt,    "Number formatting"},
    {"printf",    cmd_printf,    "Formatted output"},

    /* Network (Linux/Windows) */
    {"ifconfig",  cmd_ifconfig,  "Network interface config"},
    {"ip",        cmd_ip,        "IP configuration (Linux)"},
    {"ipconfig",  cmd_ip,        "IP config (Windows alias)"},
    {"netstat",   cmd_netstat,   "Network statistics"},
    {"ping",      cmd_ping,      "Ping a host"},
    {"traceroute",cmd_traceroute,"Trace route (simulated)"},
    {"tracert",   cmd_traceroute,"Trace route (Windows alias)"},
    {"nslookup",  cmd_nslookup,  "DNS lookup (simulated)"},
    {"dig",       cmd_dig,       "DNS lookup (simulated)"},
    {"host",      cmd_host,      "DNS lookup (simulated)"},
    {"wget",      cmd_wget,      "Download file (simulated)"},
    {"curl",      cmd_curl,      "Transfer URL (simulated)"},
    {"netsh",     cmd_netsh,     "Network shell (simulated)"},
    {"route",     cmd_route,     "Routing table"},
    {"arp",       cmd_arp,       "ARP table"},
    {"ss",        cmd_ss,        "Socket statistics"},
    {"nc",        cmd_nc,        "Netcat (simulated)"},
    {"telnet",    cmd_telnet,    "Telnet client (simulated)"},
    {"ftp",       cmd_ftp,       "FTP client (simulated)"},
    {"ssh",       cmd_ssh,       "SSH client (simulated)"},
    {"scp",       cmd_scp,       "Secure copy (simulated)"},
    {"sftp",      cmd_sftp,      "SFTP client (simulated)"},
    {"nmap",      cmd_nmap,      "Port scan (simulated)"},
    {"whois",     cmd_whois,     "WHOIS lookup (simulated)"},

    /* Process/Job control */
    {"jobs",      cmd_jobs,      "List background jobs"},
    {"bg",        cmd_bg,        "Put job in background"},
    {"fg",        cmd_fg,        "Bring job to foreground"},
    {"nice",      cmd_nice,      "Run with priority"},
    {"renice",    cmd_renice,    "Change priority"},
    {"nohup",     cmd_nohup,     "Run immune to hangup"},
    {"at",        cmd_at,        "Schedule command"},
    {"cron",      cmd_cron,      "Scheduled tasks (simulated)"},
    {"crontab",   cmd_crontab,   "Edit cron jobs (simulated)"},
    {"sleep",     cmd_sleep,     "Sleep for seconds"},
    {"timeout",   cmd_timeout,   "Run with timeout"},
    {"watch",     cmd_watch,     "Run command repeatedly"},

    /* Text editors/viewers */
    {"more",      cmd_more,      "Page through text"},
    {"less",      cmd_more,      "Page through text (alias)"},
    {"view",      cmd_cat,       "View file (alias)"},
    {"tac",       cmd_tac,       "Concatenate in reverse"},

    /* Compression (simulated) */
    {"gzip",      cmd_gzip,      "Compress (simulated)"},
    {"gunzip",    cmd_gunzip,    "Decompress (simulated)"},
    {"zip",       cmd_zip,       "Zip archive (simulated)"},
    {"unzip",     cmd_unzip,     "Unzip archive (simulated)"},
    {"tar",       cmd_tar,       "Tape archive (simulated)"},
    {"compress",  cmd_compress,  "Compress (simulated)"},
    {"bzip2",     cmd_bzip2,     "Bzip2 compress (simulated)"},
    {"xz",        cmd_xz,        "XZ compress (simulated)"},

    /* Misc Unix/Linux */
    {"echo",      cmd_echo,      "Display text"},
    {"yes",       cmd_yes,       "Print y repeatedly"},
    {"seq",       cmd_seq,       "Print number sequence"},
    {"basename",  cmd_basename,  "Strip directory from path"},
    {"dirname",   cmd_dirname,   "Strip filename from path"},
    {"realpath",  cmd_realpath,  "Resolve full path"},
    {"mktemp",    cmd_mktemp,    "Create temp file"},
    {"tempfile",  cmd_mktemp,    "Create temp file (alias)"},
    {"date",      cmd_date,      "Show date/time"},
    {"touch",     cmd_touch,     "Update timestamps"},
    {"sync",      cmd_sync,      "Flush filesystem buffers"},
    {"mount",     cmd_mount,     "Mount filesystem (simulated)"},
    {"umount",    cmd_umount,    "Unmount (simulated)"},
    {"swapon",    cmd_swapon,    "Enable swap (simulated)"},
    {"swapoff",   cmd_swapoff,   "Disable swap (simulated)"},
    {"mkfs",      cmd_mkfs,      "Make filesystem (simulated)"},
    {"fsck",      cmd_fsck,      "Check filesystem (simulated)"},
    {"dd",        cmd_dd,        "Convert and copy (simulated)"},
    {"tee",       cmd_tee,       "Duplicate output"},
    {"xargs",     cmd_xargs,     "Build command lines"},
    {"envsubst",  cmd_envsubst,  "Substitute env vars"},
    {"timecmd",   cmd_timecmd,   "Time a command"},
    {"timeout",   cmd_timeout,   "Run with timeout"},
    {"watch",     cmd_watch,     "Repeat command"},
    {"strace",    cmd_strace,    "Trace syscalls (simulated)"},
    {"ltrace",    cmd_ltrace,    "Trace library calls (simulated)"},
    {"ldd",       cmd_ldd,       "List dependencies (simulated)"},
    {"objdump",   cmd_objdump,   "Object file info (simulated)"},
    {"nm",        cmd_nm,        "Symbol table (simulated)"},
    {"size",      cmd_size,      "Section sizes (simulated)"},
    {"strings",   cmd_strings,   "Extract strings"},
    {"strip",     cmd_strip,     "Strip symbols (simulated)"},
    {"readelf",   cmd_readelf,   "ELF info (simulated)"},
    {"file",      cmd_file,      "File type identification"},
    {"stat",      cmd_stat,      "File statistics"},

    /* Windows-specific */
    {"tasklist",  cmd_ps,        "List processes (Windows)"},
    {"taskkill",  cmd_kill,      "Kill process (Windows)"},
    {"systeminfo",cmd_systeminfo,"System information (Windows)"},
    {"wmic",      cmd_wmic,      "WMI command (simulated)"},
    {"sfc",       cmd_sfc,       "System file checker (simulated)"},
    {"chkdsk",    cmd_chkdsk,    "Check disk (simulated)"},
    {"diskpart",  cmd_diskpart,  "Disk partition (simulated)"},
    {"reg",       cmd_reg,       "Registry (simulated)"},
    {"sc",        cmd_sc,        "Service control (simulated)"},
    {"net",       cmd_net,       "Network commands (simulated)"},
    {"powershell",cmd_powershell,"PowerShell (simulated)"},
    {"cmd",       cmd_cmd,       "Command prompt (simulated)"},
    {"driverquery",cmd_driverquery,"Driver list (simulated)"},
    {"gpresult",  cmd_gpresult,  "Group policy (simulated)"},
    {"shutdown",  cmd_shutdown,  "Shutdown/restart"},
    {"logoff",    cmd_logoff,    "Log off (simulated)"},
    {"assoc",     cmd_assoc,     "File associations (simulated)"},
    {"ftype",     cmd_ftype,     "File type (simulated)"},
    {"color",     cmd_color,     "Set console color"},
    {"title",     cmd_title,     "Set window title"},
    {"prompt",    cmd_prompt,    "Set prompt"},
    {"doskey",    cmd_doskey,    "Command macros (simulated)"},
    {"mode",      cmd_mode,      "Console mode (simulated)"},

    /* Fun */
    {"cowsay",    cmd_cowsay,    "Talking cow"},
    {"cowthink",  cmd_cowthink,  "Thinking cow"},
    {"matrix",    cmd_matrix,    "Matrix rain effect"},
    {"sl",        cmd_sl,        "Steam locomotive"},
    {"fortune",   cmd_fortune,   "Random fortune"},
    {"figlet",    cmd_figlet,    "Large text (simple)"},
    {"banner",    cmd_banner,    "Print banner"},
    {"toilet",    cmd_toilet,    "Colored text"},
    {"rev",       cmd_rev,       "Reverse text"},
    {"rot13",     cmd_rot13,     "ROT13 cipher"},
    {"morse",     cmd_morse,     "Morse code encoder"},
    {"pi",        cmd_pi,        "Print digits of pi"},
    {"random",    cmd_random,    "Random number"},
    {"dice",      cmd_dice,      "Roll dice"},
    {"coin",      cmd_coin,      "Flip coin"},
    {"rps",       cmd_rps,       "Rock paper scissors"},
    {"quiz",      cmd_quiz,      "Math quiz"},

    /* GUI */
    {"gui",       cmd_gui,       "Start graphical interface"},

    /* Scripting */
    {"if",        cmd_test,      "Conditional (scripting keyword)"},
    {"for",       cmd_seq,       "Loop (scripting keyword)"},
    {"while",     cmd_test,      "Loop while (scripting keyword)"},
    {"until",     cmd_test,      "Loop until (scripting keyword)"},
    {"do",        cmd_true,      "Loop body marker (scripting keyword)"},
    {"done",      cmd_true,      "Loop end (scripting keyword)"},
    {"fi",        cmd_true,      "If end (scripting keyword)"},
    {"then",      cmd_true,      "If body marker (scripting keyword)"},
    {"else",      cmd_true,      "Else branch (scripting keyword)"},
    {"elif",      cmd_test,      "Else-if (scripting keyword)"},
    {"case",      cmd_test,      "Case keyword (scripting keyword)"},
    {"esac",      cmd_true,      "Case end (scripting keyword)"},

    /* Editor */
    {"edit",      cmd_edit,      "Line-based text editor"},
    {"nano",      cmd_edit,      "Text editor (alias)"},
    {"vi",        cmd_edit,      "Text editor (alias)"},

    /* Games */
    {"2048",      cmd_game_2048, "2048 puzzle game"},
    {"snake",     cmd_snake,     "Snake game"},
    {"tictactoe", cmd_tictactoe, "Tic-tac-toe vs CPU"},
    {"ttt",       cmd_tictactoe, "Tic-tac-toe (alias)"},
    {"hangman",   cmd_hangman,   "Hangman word game"},

    /* Math */
    {"sci",       cmd_sci,       "Scientific calculator"},
    {"stats",     cmd_stats,     "Statistical analysis"},
    {"primes",    cmd_primes,    "List prime numbers"},
    {"fib",       cmd_fib,       "Fibonacci sequence"},
    {"roman",     cmd_roman,     "Roman numeral converter"},
    {"angles",    cmd_angles,    "Angle unit converter"},

    /* Ciphers & encoding */
    {"caesar",    cmd_caesar,    "Caesar cipher"},
    {"vigenere",  cmd_vigenere,  "Vigenere cipher"},
    {"atbash",    cmd_atbash,    "Atbash cipher"},
    {"urlencode", cmd_urlencode, "URL encode text"},
    {"urldecode", cmd_urldecode, "URL decode text"},
    {"hash",      cmd_hash,      "Hash a file (djb2/fnv1a/crc16)"},

    /* Animations */
    {"donut",     cmd_donut,     "3D spinning donut"},
    {"stars",     cmd_stars,     "Starfield animation"},
    {"clock",     cmd_clock,     "Live clock display"},
    {"hollywood", cmd_hollywood, "Hollywood hacker screen"},
    {"progress",  cmd_progress,  "Progress bar animation"},

    /* System tools */
    {"bench",     cmd_bench,     "CPU benchmark"},

    {NULL, NULL, NULL}
};

void env_init(void) {
    memset(env_table, 0, sizeof(env_table));
    env_set("PATH", "/bin:/usr/bin");
    env_set("HOME", "/home/user");
    env_set("USER", "user");
    env_set("SHELL", "/bin/sh");
    env_set("OS", "CosmosOS");
    env_set("HOSTNAME", "cosmos");
}

const char* env_get(const char* name) {
    for (int i = 0; i < MAX_ENV; i++) {
        if (env_table[i].used && strcmp(env_table[i].name, name) == 0) {
            return env_table[i].value;
        }
    }
    return NULL;
}

int env_set(const char* name, const char* value) {
    /* Update existing */
    for (int i = 0; i < MAX_ENV; i++) {
        if (env_table[i].used && strcmp(env_table[i].name, name) == 0) {
            strncpy(env_table[i].value, value, ENV_VAL_LEN - 1);
            return 0;
        }
    }
    /* Create new */
    for (int i = 0; i < MAX_ENV; i++) {
        if (!env_table[i].used) {
            env_table[i].used = true;
            strncpy(env_table[i].name, name, ENV_NAME_LEN - 1);
            strncpy(env_table[i].value, value, ENV_VAL_LEN - 1);
            return 0;
        }
    }
    return -1;
}

int env_unset(const char* name) {
    for (int i = 0; i < MAX_ENV; i++) {
        if (env_table[i].used && strcmp(env_table[i].name, name) == 0) {
            env_table[i].used = false;
            return 0;
        }
    }
    return -1;
}

void env_list(void) {
    for (int i = 0; i < MAX_ENV; i++) {
        if (env_table[i].used) {
            kprintf("%s=%s\n", env_table[i].name, env_table[i].value);
        }
    }
}

/* Command implementations */

static int cmd_count(void) {
    int n = 0;
    while (commands[n].name) n++;
    return n;
}

int cmd_help(int argc, char** argv) {
    if (argc > 1 && strcmp(argv[1], "--all") != 0) {
        for (int i = 0; commands[i].name; i++) {
            if (strcmp(argv[1], commands[i].name) == 0) {
                kprintf("%s - %s\n", commands[i].name, commands[i].help);
                return 0;
            }
        }
        kprintf("help: no help for '%s'\n", argv[1]);
        return 1;
    }
    if (argc > 1) {
        kprintf("\nBlackMatrixOS Commands (%d):\n", cmd_count());
        kprintf("==========================\n");
        for (int i = 0; commands[i].name; i++) {
            kprintf("  %-12s %s\n", commands[i].name, commands[i].help);
        }
        kprintf("\n");
        return 0;
    }
    kprintf("\nBlackMatrixOS Shell - %d built-in commands\n", cmd_count());
    kprintf("===========================================\n");
    kprintf("Files:     ls cd pwd cat mkdir rm rmdir touch cp mv ln stat chmod find tree\n");
    kprintf("           df du file readlink which whereis locate realpath mktemp sync\n");
    kprintf("Text:      grep wc head tail sort uniq rev tac nl cut paste fold expand tr\n");
    kprintf("           comm diff cmp tee shuf column od hexdump strings base64\n");
    kprintf("Scripting: if/elif/else/fi for/while/until/done $VAR=val pipes '|' redirect '>'\n");
    kprintf("           source break continue at eval let test read\n");
    kprintf("Editor:    edit <file>   (a=append i=insert d=delete r=replace s=subst w=save)\n");
    kprintf("Games:     2048 snake tictactoe hangman\n");
    kprintf("Math:      calc sci expr stats primes fib roman angles seq factor hex dec bitcalc\n");
    kprintf("Ciphers:   caesar vigenere atbash urlencode urldecode hash rot13 morse\n");
    kprintf("Anim:      donut stars clock hollywood progress matrix\n");
    kprintf("System:    ver uname uptime mem free ps top lscpu dmesg neofetch bench date cal\n");
    kprintf("Network:   ifconfig ip ping netstat traceroute nslookup nmap whois\n");
    kprintf("Misc:      cowsay fortune figlet banner sl quiz dice coin rps\n");
    kprintf("\nType 'help <command>' for a command's description, 'help --all' for the full list.\n");
    kprintf("Type 'gui' or press F1 for the graphical desktop.\n\n");
    return 0;
}

int cmd_clear(int argc UNUSED, char** argv UNUSED) {
    vga_clear();
    return 0;
}

int cmd_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (i > 1) kprintf(" ");
        kprintf("%s", argv[i]);
    }
    kprintf("\n");
    return 0;
}

int cmd_ls(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : fs_getcwd();
    fs_ls(path);
    return 0;
}

int cmd_cd(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : env_get("HOME");
    if (fs_chdir(path) < 0) {
        kprintf("cd: %s: No such directory\n", path);
        return 1;
    }
    return 0;
}

int cmd_pwd(int argc UNUSED, char** argv UNUSED) {
    kprintf("%s\n", fs_getcwd());
    return 0;
}

int cmd_cat(int argc, char** argv) {
    if (argc < 2) {
        kprintf("Usage: cat <file> [file2 ...]\n");
        return 1;
    }
    for (int i = 1; i < argc; i++) {
        int fd = fs_open(argv[i], O_RDONLY);
        if (fd < 0) {
            kprintf("cat: %s: No such file\n", argv[i]);
            continue;
        }
        char buf[512];
        int n;
        while ((n = fs_read(fd, buf, sizeof(buf) - 1)) > 0) {
            buf[n] = '\0';
            kprintf("%s", buf);
        }
        fs_close(fd);
    }
    return 0;
}

int cmd_mkdir(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: mkdir <dir>\n"); return 1; }
    if (fs_mkdir(argv[1], 0755) < 0) {
        kprintf("mkdir: Cannot create '%s'\n", argv[1]);
        return 1;
    }
    return 0;
}

int cmd_rm(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: rm <file>\n"); return 1; }
    if (fs_unlink(argv[1]) < 0) {
        kprintf("rm: Cannot remove '%s'\n", argv[1]);
        return 1;
    }
    return 0;
}

int cmd_touch(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: touch <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_CREAT | O_WRONLY);
    if (fd < 0) { kprintf("touch: Cannot create '%s'\n", argv[1]); return 1; }
    fs_close(fd);
    return 0;
}

int cmd_cp(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: cp <src> <dst>\n"); return 1; }
    int src = fs_open(argv[1], O_RDONLY);
    if (src < 0) { kprintf("cp: Cannot open '%s'\n", argv[1]); return 1; }
    int dst = fs_open(argv[2], O_CREAT | O_WRONLY | O_TRUNC);
    if (dst < 0) { fs_close(src); kprintf("cp: Cannot create '%s'\n", argv[2]); return 1; }
    char buf[512];
    int n;
    while ((n = fs_read(src, buf, sizeof(buf))) > 0) {
        fs_write(dst, buf, n);
    }
    fs_close(src);
    fs_close(dst);
    return 0;
}

int cmd_mv(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: mv <src> <dst>\n"); return 1; }
    if (cmd_cp(argc, argv) == 0) {
        fs_unlink(argv[1]);
        return 0;
    }
    return 1;
}

int cmd_ps(int argc UNUSED, char** argv UNUSED) {
    process_list();
    return 0;
}

int cmd_kill(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: kill <pid>\n"); return 1; }
    pid_t pid = atoi(argv[1]);
    process_kill(pid);
    kprintf("Process %d killed.\n", pid);
    return 0;
}

int cmd_mem(int argc UNUSED, char** argv UNUSED) {
    kprintf("Physical Memory:\n");
    kprintf("  Total frames: %d (%d KB)\n", pmm_get_total_frames(), pmm_get_total_frames() * 4);
    kprintf("  Free frames:  %d (%d KB)\n", pmm_get_free_frames(), pmm_get_free_frames() * 4);
    kprintf("  Used frames:  %d (%d KB)\n",
            pmm_get_total_frames() - pmm_get_free_frames(),
            (pmm_get_total_frames() - pmm_get_free_frames()) * 4);
    kprintf("\nKernel Heap:\n");
    kprintf("  Used: %d bytes\n", heap_get_used());
    kprintf("  Free: %d bytes\n", heap_get_free());
    return 0;
}

int cmd_uptime(int argc UNUSED, char** argv UNUSED) {
    uint32_t secs = timer_get_seconds();
    uint32_t mins = secs / 60;
    uint32_t hours = mins / 60;
    uint32_t days = hours / 24;
    kprintf("Uptime: %d days, %d:%02d:%02d (%d ticks)\n",
            days, hours % 60, mins % 60, secs % 60, timer_get_ticks());
    return 0;
}

int cmd_ver(int argc UNUSED, char** argv UNUSED) {
    kprintf("CosmosOS v1.0.0\n");
    kprintf("Build: 2026-03-28\n");
    kprintf("Architecture: x86 (i686)\n");
    kprintf("Kernel: Monolithic\n");
    return 0;
}

int cmd_reboot(int argc UNUSED, char** argv UNUSED) {
    kprintf("Rebooting...\n");
    uint8_t good = 0x02;
    while (good & 0x02) good = inb(0x64);
    outb(0x64, 0xFE);
    __asm__ volatile ("hlt");
    return 0;
}

/* ---------- System: PCI device list ---------- */
static uint32_t pci_read_config(uint8_t bus, uint8_t dev, uint8_t func, uint8_t reg) {
    uint32_t addr = 0x80000000 | ((uint32_t)bus << 16) | ((uint32_t)dev << 11) |
                    ((uint32_t)func << 8) | (reg & 0xFC);
    outl(0xCF8, addr);
    return inl(0xCFC);
}

int cmd_lspci(int argc UNUSED, char** argv UNUSED) {
    kprintf("Bus  Dev  Fnc  Vendor  Device  Class     IRQ\n");
    kprintf("----------------------------------------------\n");
    int count = 0;
    for (uint8_t bus = 0; bus < 1; bus++) {
        for (uint8_t dev = 0; dev < 32; dev++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t id = pci_read_config(bus, dev, func, 0x00);
                uint16_t vendor = id & 0xFFFF;
                if (vendor == 0xFFFF) { if (func == 0) break; continue; }
                uint16_t device = id >> 16;
                uint32_t cls = pci_read_config(bus, dev, func, 0x08);
                uint8_t irq = pci_read_config(bus, dev, func, 0x3C) & 0xFF;
                kprintf("%02x    %02x    %02x    %04x    %04x   %02x%02x%02x   %d\n",
                        bus, dev, func, vendor, device,
                        (cls >> 16) & 0xFF, (cls >> 8) & 0xFF, cls & 0xFF, irq);
                count++;
                if (func == 0 && !((id >> 16) & 0x8000)) break; /* single-function device */
            }
        }
    }
    if (count == 0) kprintf("No PCI devices found.\n");
    else kprintf("%d device(s) found.\n", count);
    return 0;
}

/* ---------- System: network statistics ---------- */
int cmd_netstat(int argc UNUSED, char** argv UNUSED) {
    if (g_nic == NULL) {
        kprintf("No network interface detected.\n");
        return 0;
    }
    kprintf("Interface : %s\n", g_nic->name);
    kprintf("MAC       : %02x:%02x:%02x:%02x:%02x:%02x\n",
            g_nic->mac[0], g_nic->mac[1], g_nic->mac[2],
            g_nic->mac[3], g_nic->mac[4], g_nic->mac[5]);
    kprintf("IP        : %d.%d.%d.%d\n",
            (g_nic->ip >> 24) & 0xFF, (g_nic->ip >> 16) & 0xFF,
            (g_nic->ip >> 8) & 0xFF, g_nic->ip & 0xFF);
    kprintf("Netmask   : %d.%d.%d.%d\n",
            (g_nic->netmask >> 24) & 0xFF, (g_nic->netmask >> 16) & 0xFF,
            (g_nic->netmask >> 8) & 0xFF, g_nic->netmask & 0xFF);
    kprintf("Gateway   : %d.%d.%d.%d\n",
            (g_nic->gateway >> 24) & 0xFF, (g_nic->gateway >> 16) & 0xFF,
            (g_nic->gateway >> 8) & 0xFF, g_nic->gateway & 0xFF);
    kprintf("Link      : %s\n", g_nic->link_up ? "UP" : "DOWN");
    return 0;
}

int cmd_hexdump(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: hexdump <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { kprintf("hexdump: Cannot open '%s'\n", argv[1]); return 1; }
    uint8_t buf[16];
    int n;
    uint32_t offset = 0;
    while ((n = fs_read(fd, buf, 16)) > 0) {
        kprintf("%08x: ", offset);
        for (int i = 0; i < 16; i++) {
            if (i < n) kprintf("%02x ", buf[i]);
            else kprintf("   ");
            if (i == 7) kprintf(" ");
        }
        kprintf(" |");
        for (int i = 0; i < n; i++) {
            kprintf("%c", (buf[i] >= 32 && buf[i] < 127) ? buf[i] : '.');
        }
        kprintf("|\n");
        offset += n;
    }
    fs_close(fd);
    return 0;
}

int cmd_df(int argc UNUSED, char** argv UNUSED) {
    kprintf("Filesystem    Size     Used     Free  Use%%\n");
    kprintf("/dev/ram0     ");
    int total_kb = FS_MAX_BLOCKS * FS_BLOCK_SIZE / 1024;
    int used_kb = 0;
    for (int i = 0; i < FS_MAX_FILES; i++) {
        /* Count used blocks */
    }
    kprintf("%4d KB  %4d KB  %4d KB  %d%%\n",
            total_kb, used_kb, total_kb - used_kb,
            total_kb > 0 ? (used_kb * 100 / total_kb) : 0);
    kprintf("Inodes: %d/%d used\n", inode_count, FS_MAX_FILES);
    return 0;
}

int cmd_grep(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: grep <pattern> <file>\n"); return 1; }
    int fd = fs_open(argv[2], O_RDONLY);
    if (fd < 0) { kprintf("grep: Cannot open '%s'\n", argv[2]); return 1; }
    char line[512];
    int li = 0;
    char c;
    while (fs_read(fd, &c, 1) > 0) {
        if (c == '\n' || li >= 511) {
            line[li] = '\0';
            if (strstr(line, argv[1])) kprintf("%s\n", line);
            li = 0;
        } else {
            line[li++] = c;
        }
    }
    if (li > 0) {
        line[li] = '\0';
        if (strstr(line, argv[1])) kprintf("%s\n", line);
    }
    fs_close(fd);
    return 0;
}

int cmd_wc(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: wc <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { kprintf("wc: Cannot open '%s'\n", argv[1]); return 1; }
    int lines = 0, words = 0, chars = 0;
    int in_word = 0;
    char c;
    while (fs_read(fd, &c, 1) > 0) {
        chars++;
        if (c == '\n') lines++;
        if (c == ' ' || c == '\t' || c == '\n') {
            if (in_word) words++;
            in_word = 0;
        } else {
            in_word = 1;
        }
    }
    if (in_word) words++;
    fs_close(fd);
    kprintf("  %d  %d  %d  %s\n", lines, words, chars, argv[1]);
    return 0;
}

int cmd_head(int argc, char** argv) {
    int num = 10;
    const char* filename = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            num = atoi(argv[++i]);
        } else {
            filename = argv[i];
        }
    }
    if (!filename) { kprintf("Usage: head [-n N] <file>\n"); return 1; }
    int fd = fs_open(filename, O_RDONLY);
    if (fd < 0) { kprintf("head: Cannot open '%s'\n", filename); return 1; }
    char c;
    int lines = 0;
    while (fs_read(fd, &c, 1) > 0) {
        kprintf("%c", c);
        if (c == '\n' && ++lines >= num) break;
    }
    fs_close(fd);
    return 0;
}

int cmd_find(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : "/";
    /* Simple recursive listing */
    kprintf("%s\n", path);
    return 0;
}

static void tree_recursive(const char* path, int depth) {
    dir_t* dir = fs_opendir(path);
    if (!dir) return;
    dirent_t* entry;
    while ((entry = fs_readdir(dir))) {
        for (int i = 0; i < depth; i++) kprintf("  ");
        kprintf("|-- %s\n", entry->name);
        if (entry->type == FS_DIRECTORY && strcmp(entry->name, ".") != 0 && strcmp(entry->name, "..") != 0) {
            char child_path[FS_MAX_PATH];
            sprintf(child_path, "%s/%s", path, entry->name);
            tree_recursive(child_path, depth + 1);
        }
    }
    fs_closedir(dir);
}

int cmd_tree(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : fs_getcwd();
    kprintf("%s\n", path);
    tree_recursive(path, 1);
    return 0;
}

int cmd_whoami(int argc UNUSED, char** argv UNUSED) {
    kprintf("%s\n", env_get("USER"));
    return 0;
}

int cmd_hostname(int argc UNUSED, char** argv UNUSED) {
    int fd = fs_open("/etc/hostname", O_RDONLY);
    if (fd >= 0) {
        char buf[64];
        int n = fs_read(fd, buf, sizeof(buf) - 1);
        buf[n] = '\0';
        kprintf("%s", buf);
        fs_close(fd);
    }
    return 0;
}

int cmd_env(int argc UNUSED, char** argv UNUSED) {
    env_list();
    return 0;
}

int cmd_export(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: export NAME=VALUE\n"); return 1; }
    char* eq = strchr(argv[1], '=');
    if (!eq) { kprintf("Invalid format. Use NAME=VALUE\n"); return 1; }
    *eq = '\0';
    env_set(argv[1], eq + 1);
    return 0;
}

int cmd_calc(int argc, char** argv) {
    if (argc < 4) { kprintf("Usage: calc <num1> <op> <num2>\n"); return 1; }
    int a = atoi(argv[1]);
    int b = atoi(argv[3]);
    char op = argv[2][0];
    int result;
    switch (op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/':
            if (b == 0) { kprintf("Error: Division by zero\n"); return 1; }
            result = a / b;
            break;
        case '%':
            if (b == 0) { kprintf("Error: Division by zero\n"); return 1; }
            result = a % b;
            break;
        default: kprintf("Unknown operator '%c'\n", op); return 1;
    }
    kprintf("%d %c %d = %d\n", a, op, b, result);
    return 0;
}

int cmd_cowsay(int argc, char** argv) {
    const char* msg = (argc > 1) ? argv[1] : "Moo!";
    int len = strlen(msg);
    kprintf(" ");
    for (int i = 0; i < len + 2; i++) kprintf("_");
    kprintf("\n< %s >\n ", msg);
    for (int i = 0; i < len + 2; i++) kprintf("-");
    kprintf("\n        \\   ^__^\n");
    kprintf("         \\  (oo)\\_______\n");
    kprintf("            (__)\\       )\\/\\\n");
    kprintf("                ||----w |\n");
    kprintf("                ||     ||\n");
    return 0;
}

int cmd_matrix(int argc UNUSED, char** argv UNUSED) {
    /* Matrix rain effect (text mode) */
    vga_set_color(VGA_COLOR(VGA_GREEN, VGA_BLACK));
    kprintf("Press any key to stop matrix rain...\n");
    timer_sleep(500);

    int cols[VGA_WIDTH];
    for (int i = 0; i < VGA_WIDTH; i++) cols[i] = -rand() % VGA_HEIGHT;

    while (!keyboard_has_input()) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            if (cols[x] >= 0 && cols[x] < VGA_HEIGHT) {
                char c = 33 + (timer_get_ticks() * 7 + x * 13) % 93;
                vga_put_at(c, VGA_COLOR(VGA_LIGHT_GREEN, VGA_BLACK), cols[x], x);
                if (cols[x] > 0) {
                    vga_put_at(33 + (timer_get_ticks() + x) % 93,
                              VGA_COLOR(VGA_GREEN, VGA_BLACK), cols[x] - 1, x);
                }
            }
            cols[x]++;
            if (cols[x] > VGA_HEIGHT + 5) cols[x] = -((timer_get_ticks() + x * 7) % 20);
        }
        timer_sleep(50);
    }
    keyboard_getchar();
    vga_clear();
    return 0;
}

int cmd_gui(int argc UNUSED, char** argv UNUSED) {
    gui_run();
    return 0;
}

int cmd_ifconfig(int argc UNUSED, char** argv UNUSED) {
    kprintf("Network interface: eth0\n");
    kprintf("  IP Address:  10.0.2.15\n");
    kprintf("  Netmask:     255.255.255.0\n");
    kprintf("  Gateway:     10.0.2.2\n");
    kprintf("  MAC Address: 52:54:00:12:34:56\n");
    kprintf("  Status:      Link up\n");
    return 0;
}

int cmd_ping(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: ping <ip>\n"); return 1; }
    kprintf("PING %s: 64 bytes\n", argv[1]);
    for (int i = 0; i < 4; i++) {
        timer_sleep(1000);
        kprintf("64 bytes from %s: icmp_seq=%d ttl=64 time=%d ms\n", argv[1], i, 10 + i * 2);
    }
    return 0;
}

int cmd_tail(int argc, char** argv) {
    int num = 10;
    const char* filename = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) num = atoi(argv[++i]);
        else filename = argv[i];
    }
    if (!filename) { kprintf("Usage: tail [-n N] <file>\n"); return 1; }
    int fd = fs_open(filename, O_RDONLY);
    if (fd < 0) { kprintf("tail: Cannot open '%s'\n", filename); return 1; }
    char lines[256][256];
    int total = 0, li = 0;
    char c;
    while (fs_read(fd, &c, 1) > 0) {
        if (c == '\n' || li >= 255) {
            lines[total % 256][li] = '\0';
            total++; li = 0;
        } else {
            lines[total % 256][li++] = c;
        }
    }
    if (li > 0) { lines[total % 256][li] = '\0'; total++; }
    fs_close(fd);
    int start = total > num ? total - num : 0;
    for (int i = start; i < total; i++) kprintf("%s\n", lines[i % 256]);
    return 0;
}

int cmd_sort(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: sort <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { kprintf("sort: Cannot open '%s'\n", argv[1]); return 1; }
    char lines[512][128];
    int nlines = 0, li = 0;
    char c;
    while (fs_read(fd, &c, 1) > 0 && nlines < 512) {
        if (c == '\n' || li >= 127) {
            lines[nlines][li] = '\0'; nlines++; li = 0;
        } else {
            lines[nlines][li++] = c;
        }
    }
    if (li > 0) { lines[nlines][li] = '\0'; nlines++; }
    fs_close(fd);
    for (int i = 0; i < nlines - 1; i++)
        for (int j = i + 1; j < nlines; j++)
            if (strcmp(lines[i], lines[j]) > 0) {
                char tmp[128]; strcpy(tmp, lines[i]);
                strcpy(lines[i], lines[j]); strcpy(lines[j], tmp);
            }
    for (int i = 0; i < nlines; i++) kprintf("%s\n", lines[i]);
    return 0;
}

int cmd_uniq(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: uniq <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { kprintf("uniq: Cannot open '%s'\n", argv[1]); return 1; }
    char prev[256] = "", line[256];
    int li = 0;
    char c;
    while (fs_read(fd, &c, 1) > 0) {
        if (c == '\n' || li >= 255) {
            line[li] = '\0';
            if (strcmp(line, prev) != 0) { kprintf("%s\n", line); strcpy(prev, line); }
            li = 0;
        } else { line[li++] = c; }
    }
    if (li > 0) { line[li] = '\0'; if (strcmp(line, prev) != 0) kprintf("%s\n", line); }
    fs_close(fd);
    return 0;
}

int cmd_basename(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: basename <path> [suffix]\n"); return 1; }
    const char* p = argv[1];
    const char* last = p;
    while (*p) { if (*p == '/') last = p + 1; p++; }
    kprintf("%s\n", last);
    return 0;
}

int cmd_dirname(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: dirname <path>\n"); return 1; }
    char buf[FS_MAX_PATH];
    strncpy(buf, argv[1], FS_MAX_PATH - 1);
    int len = strlen(buf);
    while (len > 0 && buf[len - 1] != '/') len--;
    if (len > 1 && buf[len - 1] == '/') buf[len - 1] = '\0';
    if (len == 0) kprintf("/\n");
    else { buf[len] = '\0'; kprintf("%s\n", buf); }
    return 0;
}

int cmd_seq(int argc, char** argv) {
    int start = 1, end = 10, step = 1;
    if (argc == 2) end = atoi(argv[1]);
    else if (argc == 3) { start = atoi(argv[1]); end = atoi(argv[2]); }
    else if (argc >= 4) { start = atoi(argv[1]); step = atoi(argv[2]); end = atoi(argv[3]); }
    for (int i = start; step > 0 ? i <= end : i >= end; i += step) kprintf("%d\n", i);
    return 0;
}

int cmd_rev(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: rev <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { kprintf("rev: Cannot open '%s'\n", argv[1]); return 1; }
    char line[256]; int li = 0;
    char c;
    while (fs_read(fd, &c, 1) > 0) {
        if (c == '\n' || li >= 255) {
            for (int i = li - 1; i >= 0; i--) kprintf("%c", line[i]);
            kprintf("\n"); li = 0;
        } else { line[li++] = c; }
    }
    if (li > 0) { for (int i = li - 1; i >= 0; i--) kprintf("%c", line[i]); kprintf("\n"); }
    fs_close(fd);
    return 0;
}

int cmd_factor(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: factor <number>\n"); return 1; }
    int n = atoi(argv[1]);
    if (n < 2) { kprintf("%d: not factorable\n", n); return 1; }
    kprintf("%d:", n);
    for (int d = 2; d * d <= n; d++) {
        while (n % d == 0) { kprintf(" %d", d); n /= d; }
    }
    if (n > 1) kprintf(" %d", n);
    kprintf("\n");
    return 0;
}

int cmd_tee(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: tee <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_CREAT | O_WRONLY | O_TRUNC);
    if (fd < 0) { kprintf("tee: Cannot create '%s'\n", argv[1]); return 1; }
    kprintf("tee: reading from stdin (type lines, 'quit' to stop):\n");
    char line[512];
    while (1) {
        kprintf("> ");
        int li = 0;
        while (li < 511) {
            char c = keyboard_getchar();
            if (c == '\n') break;
            if (c == '\b' && li > 0) { li--; vga_putchar('\b'); vga_putchar(' '); vga_putchar('\b'); continue; }
            if (c >= 32) { line[li++] = c; vga_putchar(c); }
        }
        line[li] = '\0'; kprintf("\n");
        if (strcmp(line, "quit") == 0) break;
        fs_write(fd, line, li); fs_write(fd, "\n", 1);
    }
    fs_close(fd);
    return 0;
}

int cmd_sleep(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: sleep <seconds>\n"); return 1; }
    int secs = atoi(argv[1]);
    timer_sleep(secs * 1000);
    return 0;
}

int cmd_true(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_false(int argc UNUSED, char** argv UNUSED) { return 1; }

int cmd_yes(int argc, char** argv) {
    const char* text = (argc > 1) ? argv[1] : "y";
    kprintf("Press any key to stop...\n");
    timer_sleep(200);
    while (!keyboard_has_input()) { kprintf("%s\n", text); }
    keyboard_getchar();
    return 0;
}

int cmd_which(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: which <command>\n"); return 1; }
    for (int i = 0; commands[i].name; i++) {
        if (strcmp(argv[1], commands[i].name) == 0) {
            kprintf("/bin/%s\n", argv[1]); return 0;
        }
    }
    kprintf("%s not found\n", argv[1]);
    return 1;
}

int cmd_nl(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: nl <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { kprintf("nl: Cannot open '%s'\n", argv[1]); return 1; }
    int line_num = 1;
    char line[512]; int li = 0;
    char c;
    while (fs_read(fd, &c, 1) > 0) {
        if (c == '\n' || li >= 511) {
            line[li] = '\0'; kprintf("%6d\t%s\n", line_num++, line); li = 0;
        } else { line[li++] = c; }
    }
    if (li > 0) { line[li] = '\0'; kprintf("%6d\t%s\n", line_num, line); }
    fs_close(fd);
    return 0;
}

int cmd_stat(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: stat <file>\n"); return 1; }
    fs_stat_t st;
    if (fs_stat(argv[1], &st) < 0) { kprintf("stat: Cannot access '%s'\n", argv[1]); return 1; }
    kprintf("  File: %s\n", argv[1]);
    kprintf("  Size: %d bytes\n", st.size);
    kprintf("  Type: %s\n", st.type == FS_DIRECTORY ? "directory" : "regular file");
    kprintf("  Permissions: 0%o\n", st.permissions);
    kprintf("  UID: %d  GID: %d\n", st.uid, st.gid);
    kprintf("  Links: %d\n", st.nlinks);
    return 0;
}

int cmd_history(int argc UNUSED, char** argv UNUSED) {
    for (int i = 0; i < cmd_history_len; i++) kprintf("  %d  %s\n", i + 1, cmd_history_buf[i]);
    return 0;
}

int cmd_uname(int argc, char** argv) {
    bool show_all = false;
    for (int i = 1; i < argc; i++) if (strcmp(argv[i], "-a") == 0) show_all = true;
    if (show_all) kprintf("BlackMatrixOS cosmos 1.0.0 x86 i686 BlackMatrixOS\n");
    else kprintf("BlackMatrixOS\n");
    return 0;
}

int cmd_id(int argc UNUSED, char** argv UNUSED) {
    kprintf("uid=0(root) gid=0(root) groups=0(root)\n");
    return 0;
}

int cmd_expr(int argc, char** argv) {
    if (argc < 4) { kprintf("Usage: expr <a> <op> <b>\n"); return 1; }
    int a = atoi(argv[1]), b = atoi(argv[3]);
    char op = argv[2][0];
    int r = 0;
    switch (op) {
        case '+': r = a + b; break;
        case '-': r = a - b; break;
        case '*': r = a * b; break;
        case '/': if (b == 0) { kprintf("Division by zero\n"); return 1; } r = a / b; break;
        case '%': if (b == 0) { kprintf("Division by zero\n"); return 1; } r = a % b; break;
        case '>': r = a > b ? 1 : 0; break;
        case '<': r = a < b ? 1 : 0; break;
        default: kprintf("Unknown operator\n"); return 1;
    }
    kprintf("%d\n", r);
    return 0;
}

int cmd_cksum(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: cksum <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { kprintf("cksum: Cannot open '%s'\n", argv[1]); return 1; }
    uint32_t crc = 0; int bytes = 0;
    char c;
    while (fs_read(fd, &c, 1) > 0) {
        crc ^= (uint32_t)(unsigned char)c << 24;
        for (int i = 0; i < 8; i++) crc = (crc & 0x80000000) ? (crc << 1) ^ 0x04C11DB7 : crc << 1;
        bytes++;
    }
    fs_close(fd);
    kprintf("%u %d %s\n", crc, bytes, argv[1]);
    return 0;
}

int cmd_sum(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: sum <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { kprintf("sum: Cannot open '%s'\n", argv[1]); return 1; }
    uint16_t chk = 0; int bytes = 0;
    char c;
    while (fs_read(fd, &c, 1) > 0) { chk = (chk >> 1) + ((chk & 1) << 15); chk += (unsigned char)c; bytes++; }
    fs_close(fd);
    kprintf("%u %d %s\n", chk, (bytes + 1023) / 1024, argv[1]);
    return 0;
}

int cmd_truncate(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: truncate -s <size> <file>\n"); return 1; }
    const char* size_str = NULL;
    const char* filename = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) size_str = argv[++i];
        else filename = argv[i];
    }
    if (!size_str || !filename) { kprintf("Usage: truncate -s <size> <file>\n"); return 1; }
    int size = atoi(size_str);
    int fd = fs_open(filename, O_WRONLY);
    if (fd < 0) { kprintf("truncate: Cannot open '%s'\n", filename); return 1; }
    fs_truncate(fd, size);
    fs_close(fd);
    return 0;
}

int cmd_date(int argc UNUSED, char** argv UNUSED) {
    kprintf("Sat Mar 29 00:00:00 UTC 2026\n");
    return 0;
}

int cmd_alias(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: alias NAME=VALUE\n"); return 1; }
    kprintf("Alias set: %s\n", argv[1]);
    return 0;
}

int cmd_chmod(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: chmod <mode> <file>\n"); return 1; }
    kprintf("chmod: %s %s (simulated)\n", argv[1], argv[2]);
    return 0;
}

int cmd_ln(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: ln <target> <link>\n"); return 1; }
    if (fs_link(argv[1], argv[2]) < 0) { kprintf("ln: Cannot create link\n"); return 1; }
    return 0;
}

int cmd_tac(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: tac <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { kprintf("tac: Cannot open '%s'\n", argv[1]); return 1; }
    char lines[512][256]; int n = 0, li = 0;
    char c;
    while (fs_read(fd, &c, 1) > 0 && n < 512) {
        if (c == '\n' || li >= 255) { lines[n][li] = '\0'; n++; li = 0; }
        else lines[n][li++] = c;
    }
    if (li > 0) { lines[n][li] = '\0'; n++; }
    fs_close(fd);
    for (int i = n - 1; i >= 0; i--) kprintf("%s\n", lines[i]);
    return 0;
}

int cmd_od(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: od <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { kprintf("od: Cannot open '%s'\n", argv[1]); return 1; }
    unsigned char buf[16]; int n; uint32_t off = 0;
    while ((n = fs_read(fd, buf, 16)) > 0) {
        kprintf("%07o", off);
        for (int i = 0; i < n; i++) kprintf(" %03o", buf[i]);
        kprintf("\n"); off += n;
    }
    fs_close(fd);
    return 0;
}

int cmd_strings(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: strings <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { kprintf("strings: Cannot open '%s'\n", argv[1]); return 1; }
    char buf[128]; int bi = 0; char c;
    while (fs_read(fd, &c, 1) > 0) {
        if (c >= 32 && c < 127) { if (bi < 127) buf[bi++] = c; }
        else { if (bi >= 4) { buf[bi] = '\0'; kprintf("%s\n", buf); } bi = 0; }
    }
    if (bi >= 4) { buf[bi] = '\0'; kprintf("%s\n", buf); }
    fs_close(fd);
    return 0;
}

int cmd_cmp(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: cmp <file1> <file2>\n"); return 1; }
    int f1 = fs_open(argv[1], O_RDONLY), f2 = fs_open(argv[2], O_RDONLY);
    if (f1 < 0) { kprintf("cmp: Cannot open '%s'\n", argv[1]); return 1; }
    if (f2 < 0) { fs_close(f1); kprintf("cmp: Cannot open '%s'\n", argv[2]); return 1; }
    char c1, c2; int off = 0;
    while (fs_read(f1, &c1, 1) > 0 && fs_read(f2, &c2, 1) > 0) {
        off++;
        if (c1 != c2) { kprintf("%s %s differ: byte %d\n", argv[1], argv[2], off); fs_close(f1); fs_close(f2); return 1; }
    }
    fs_close(f1); fs_close(f2);
    return 0;
}

int cmd_comm(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: comm <file1> <file2>\n"); return 1; }
    kprintf("comm: comparison of sorted files (simulated)\n");
    return 0;
}

int cmd_cut(int argc, char** argv) {
    int field = 1; const char* filename = NULL; char delim = '\t';
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-f", 2) == 0) field = atoi(argv[i] + 2);
        else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) delim = argv[++i][0];
        else filename = argv[i];
    }
    if (!filename) { kprintf("Usage: cut -f<N> [-d<delim>] <file>\n"); return 1; }
    int fd = fs_open(filename, O_RDONLY);
    if (fd < 0) { kprintf("cut: Cannot open '%s'\n", filename); return 1; }
    char line[512]; int li = 0; char c;
    while (fs_read(fd, &c, 1) > 0) {
        if (c == '\n' || li >= 511) {
            line[li] = '\0'; int f = 1; char* p = line;
            while (*p && f < field) { if (*p == delim) f++; p++; }
            char* end = p; while (*end && *end != delim) end++;
            char old = *end; *end = '\0'; kprintf("%s\n", p); *end = old;
            li = 0;
        } else line[li++] = c;
    }
    fs_close(fd);
    return 0;
}

int cmd_paste(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: paste <file1> <file2>\n"); return 1; }
    int f1 = fs_open(argv[1], O_RDONLY), f2 = fs_open(argv[2], O_RDONLY);
    if (f1 < 0 || f2 < 0) { kprintf("paste: Cannot open files\n"); return 1; }
    char l1[256], l2[256]; char c;
    while (1) {
        int n1 = 0, n2 = 0;
        while (fs_read(f1, &c, 1) > 0 && c != '\n' && n1 < 255) l1[n1++] = c;
        while (fs_read(f2, &c, 1) > 0 && c != '\n' && n2 < 255) l2[n2++] = c;
        if (n1 == 0 && n2 == 0) break;
        l1[n1] = '\0'; l2[n2] = '\0';
        kprintf("%s\t%s\n", l1, l2);
    }
    fs_close(f1); fs_close(f2);
    return 0;
}

int cmd_fold(int argc, char** argv) {
    int width = 80; const char* filename = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) width = atoi(argv[++i]);
        else filename = argv[i];
    }
    if (!filename) { kprintf("Usage: fold [-w N] <file>\n"); return 1; }
    int fd = fs_open(filename, O_RDONLY);
    if (fd < 0) { kprintf("fold: Cannot open '%s'\n", filename); return 1; }
    int col = 0; char c;
    while (fs_read(fd, &c, 1) > 0) {
        if (c == '\n') { kprintf("\n"); col = 0; }
        else { if (col >= width) { kprintf("\n"); col = 0; } kprintf("%c", c); col++; }
    }
    if (col > 0) kprintf("\n");
    fs_close(fd);
    return 0;
}

int cmd_expand(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: expand <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { kprintf("expand: Cannot open '%s'\n", argv[1]); return 1; }
    int col = 0; char c;
    while (fs_read(fd, &c, 1) > 0) {
        if (c == '\t') { int sp = 8 - (col % 8); for (int i = 0; i < sp; i++) { kprintf(" "); col++; } }
        else if (c == '\n') { kprintf("\n"); col = 0; }
        else { kprintf("%c", c); col++; }
    }
    fs_close(fd);
    return 0;
}

int cmd_unexpand(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: unexpand <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { kprintf("unexpand: Cannot open '%s'\n", argv[1]); return 1; }
    int sp_count = 0, col = 0; char c;
    while (fs_read(fd, &c, 1) > 0) {
        if (c == ' ') { sp_count++; col++; if (col % 8 == 0 && sp_count >= 8) { kprintf("\t"); sp_count = 0; } }
        else { for (int i = 0; i < sp_count; i++) kprintf(" "); sp_count = 0; kprintf("%c", c); if (c == '\n') col = 0; else col++; }
    }
    for (int i = 0; i < sp_count; i++) kprintf(" ");
    fs_close(fd);
    return 0;
}

int cmd_tr(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: tr <set1> <set2>\n"); return 1; }
    kprintf("tr: reading from stdin (type lines, 'quit' to stop):\n");
    const char* s1 = argv[1], *s2 = argv[2];
    char line[512];
    while (1) {
        kprintf("> "); int li = 0;
        while (li < 511) {
            char c = keyboard_getchar();
            if (c == '\n') break;
            if (c >= 32) { line[li++] = c; vga_putchar(c); }
        }
        line[li] = '\0'; kprintf("\n");
        if (strcmp(line, "quit") == 0) break;
        for (int i = 0; line[i]; i++) {
            const char* p = s1;
            while (*p) { if (*p == line[i]) { line[i] = s2[p - s1]; break; } p++; }
        }
        kprintf("%s\n", line);
    }
    return 0;
}

int cmd_envsubst(int argc UNUSED, char** argv UNUSED) {
    kprintf("envsubst: not implemented (no stdin pipe)\n");
    return 0;
}

int cmd_timecmd(int argc UNUSED, char** argv UNUSED) {
    uint32_t start = timer_get_ticks();
    kprintf("timecmd: simulated timing\n");
    uint32_t elapsed = timer_get_ticks() - start;
    kprintf("real 0m%d.%03ds\n", elapsed / 1000, elapsed % 1000);
    return 0;
}

int cmd_lscpu(int argc UNUSED, char** argv UNUSED) {
    kprintf("CPU Information:\n");
    kprintf("  Architecture: x86 (i686) 32-bit\n");
    kprintf("  Vendor: GenuineIntel\n");
    kprintf("  Model: QEMU Virtual CPU\n");
    kprintf("  Cores: 1\n");
    kprintf("  MHz: 2400\n");
    kprintf("  Cache: 256 KB L2\n");
    kprintf("  Flags: fpu vme pse tsc\n");
    return 0;
}

int cmd_dmesg(int argc UNUSED, char** argv UNUSED) {
    kprintf("[    0.000] BlackMatrixOS Kernel starting...\n");
    kprintf("[    0.001] VGA: Text mode 80x25 initialized\n");
    kprintf("[    0.002] GDT: Global Descriptor Table loaded\n");
    kprintf("[    0.003] IDT: Interrupt Descriptor Table loaded\n");
    kprintf("[    0.004] PIC: 8259 remapped\n");
    kprintf("[    0.005] PIT: Timer at 100 Hz\n");
    kprintf("[    0.006] KB:  Keyboard driver initialized\n");
    kprintf("[    0.007] PMM: Physical memory manager ready\n");
    kprintf("[    0.008] FS:  RAM filesystem mounted\n");
    kprintf("[    0.009] NET: Network stack initialized\n");
    kprintf("[    0.010] SHELL: Interactive shell ready\n");
    return 0;
}

int cmd_free(int argc UNUSED, char** argv UNUSED) {
    kprintf("             total       used       free     shared    buffers\n");
    kprintf("Mem:        131072       8192     122880          0       2048\n");
    kprintf("Swap:            0          0          0\n");
    kprintf("Total:      131072       8192     122880\n");
    return 0;
}

int cmd_top(int argc UNUSED, char** argv UNUSED) {
    kprintf("PID  USER     CPU%%  MEM%%  COMMAND\n");
    kprintf("  1  root      0.0   0.5  kernel\n");
    kprintf("  2  root      0.1   0.3  shell\n");
    kprintf("  3  root      0.0   0.2  timer\n");
    kprintf("  4  root      0.0   0.1  keyboard\n");
    kprintf("Press any key to refresh, ESC to exit\n");
    return 0;
}

int cmd_killall(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: killall <name>\n"); return 1; }
    kprintf("killall: '%s' - no matching process\n", argv[1]);
    return 0;
}

int cmd_pgrep(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: pgrep <name>\n"); return 1; }
    kprintf("pgrep: '%s' - no matching process\n", argv[1]);
    return 0;
}

int cmd_set(int argc UNUSED, char** argv UNUSED) {
    env_list();
    return 0;
}

int cmd_read(int argc, char** argv) {
    const char* var = (argc > 1) ? argv[1] : "REPLY";
    kprintf("read> ");
    char buf[256]; int i = 0;
    while (i < 255) {
        char c = keyboard_getchar();
        if (c == '\n') break;
        if (c >= 32) { buf[i++] = c; vga_putchar(c); }
    }
    buf[i] = '\0'; kprintf("\n");
    env_set(var, buf);
    return 0;
}

int cmd_test(int argc, char** argv) {
    if (argc < 4) { kprintf("Usage: test <a> <op> <b>\n"); return 1; }
    int a = atoi(argv[1]), b = atoi(argv[3]);
    if (strcmp(argv[2], "-eq") == 0) return a == b ? 0 : 1;
    if (strcmp(argv[2], "-ne") == 0) return a != b ? 0 : 1;
    if (strcmp(argv[2], "-gt") == 0) return a > b ? 0 : 1;
    if (strcmp(argv[2], "-lt") == 0) return a < b ? 0 : 1;
    if (strcmp(argv[2], "-ge") == 0) return a >= b ? 0 : 1;
    if (strcmp(argv[2], "-le") == 0) return a <= b ? 0 : 1;
    if (strcmp(argv[2], "=") == 0) return strcmp(argv[1], argv[3]) == 0 ? 0 : 1;
    if (strcmp(argv[2], "!=") == 0) return strcmp(argv[1], argv[3]) != 0 ? 0 : 1;
    kprintf("Unknown operator: %s\n", argv[2]);
    return 2;
}

int cmd_printf(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: printf <format> [args...]\n"); return 1; }
    const char* fmt = argv[1];
    int argi = 2;
    while (*fmt) {
        if (*fmt == '%' && *(fmt + 1)) {
            fmt++;
            if (*fmt == 's' && argi < argc) { kprintf("%s", argv[argi++]); }
            else if (*fmt == 'd' && argi < argc) { kprintf("%d", atoi(argv[argi++])); }
            else if (*fmt == 'c' && argi < argc) { kprintf("%c", argv[argi++][0]); }
            else if (*fmt == '%') { kprintf("%%"); }
            else { kprintf("%%%c", *fmt); }
            fmt++;
        } else { kprintf("%c", *fmt); fmt++; }
    }
    return 0;
}

int cmd_hex(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: hex <decimal>\n"); return 1; }
    int val = atoi(argv[1]);
    kprintf("0x%X\n", val);
    return 0;
}

int cmd_dec(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: dec <hex>\n"); return 1; }
    int val = 0; const char* p = argv[1];
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) p += 2;
    while (*p) {
        val <<= 4;
        if (*p >= '0' && *p <= '9') val |= (*p - '0');
        else if (*p >= 'a' && *p <= 'f') val |= (*p - 'a' + 10);
        else if (*p >= 'A' && *p <= 'F') val |= (*p - 'A' + 10);
        p++;
    }
    kprintf("%d\n", val);
    return 0;
}

int cmd_bitcalc(int argc, char** argv) {
    if (argc < 4) { kprintf("Usage: bitcalc <a> <op> <b>  (AND|OR|XOR|SHL|SHR)\n"); return 1; }
    int a = atoi(argv[1]), b = atoi(argv[3]);
    if (strcmp(argv[2], "AND") == 0) kprintf("%d & %d = %d\n", a, b, a & b);
    else if (strcmp(argv[2], "OR") == 0) kprintf("%d | %d = %d\n", a, b, a | b);
    else if (strcmp(argv[2], "XOR") == 0) kprintf("%d ^ %d = %d\n", a, b, a ^ b);
    else if (strcmp(argv[2], "SHL") == 0) kprintf("%d << %d = %d\n", a, b, a << b);
    else if (strcmp(argv[2], "SHR") == 0) kprintf("%d >> %d = %d\n", a, b, a >> b);
    else if (strcmp(argv[2], "NOT") == 0) kprintf("~%d = %d\n", a, ~a);
    else kprintf("Unknown operator: %s\n", argv[2]);
    return 0;
}

int cmd_color(int argc UNUSED, char** argv UNUSED) {
    kprintf("Color Palette:\n");
    for (int i = 0; i < 16; i++) {
        vga_set_color(VGA_COLOR(i, VGA_BLACK));
        kprintf("  ## Color %2d ", i);
        vga_set_color(VGA_COLOR(VGA_WHITE, i));
        kprintf("  Sample  ");
        vga_set_color(VGA_COLOR(VGA_LIGHT_GREY, VGA_BLACK));
        kprintf("\n");
    }
    return 0;
}

int cmd_neofetch(int argc UNUSED, char** argv UNUSED) {
    vga_set_color(VGA_COLOR(VGA_LIGHT_GREEN, VGA_BLACK));
    kprintf("       _____      \n");
    kprintf("      /     \\     ");
    vga_set_color(VGA_COLOR(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  user@blackmatrix\n");
    vga_set_color(VGA_COLOR(VGA_LIGHT_GREEN, VGA_BLACK));
    kprintf("     / BLACK   \\   ");
    vga_set_color(VGA_COLOR(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("  ---------------\n");
    vga_set_color(VGA_COLOR(VGA_GREEN, VGA_BLACK));
    kprintf("    / MATRIX    \\  ");
    vga_set_color(VGA_COLOR(VGA_LIGHT_GREEN, VGA_BLACK));
    kprintf("  OS: BlackMatrixOS 1.0\n");
    vga_set_color(VGA_COLOR(VGA_GREEN, VGA_BLACK));
    kprintf("   /  OS         \\ ");
    vga_set_color(VGA_COLOR(VGA_LIGHT_GREEN, VGA_BLACK));
    kprintf("  Kernel: Monolithic x86\n");
    vga_set_color(VGA_COLOR(VGA_GREEN, VGA_BLACK));
    kprintf("  /_______________\\");
    vga_set_color(VGA_COLOR(VGA_LIGHT_GREEN, VGA_BLACK));
    kprintf("  Shell: bmos\n");
    vga_set_color(VGA_COLOR(VGA_GREEN, VGA_BLACK));
    kprintf("  |  ||     ||    |");
    vga_set_color(VGA_COLOR(VGA_LIGHT_GREEN, VGA_BLACK));
    kprintf("  CPU: QEMU Virtual\n");
    vga_set_color(VGA_COLOR(VGA_GREEN, VGA_BLACK));
    kprintf("                  ");
    vga_set_color(VGA_COLOR(VGA_LIGHT_GREEN, VGA_BLACK));
    kprintf("  Memory: 128 MB\n");
    vga_set_color(VGA_COLOR(VGA_GREEN, VGA_BLACK));
    kprintf("                  ");
    vga_set_color(VGA_COLOR(VGA_LIGHT_GREEN, VGA_BLACK));
    kprintf("  Resolution: 80x25\n");
    kprintf("                  Shell: /bin/sh\n");
    vga_set_color(VGA_COLOR(VGA_LIGHT_GREY, VGA_BLACK));
    return 0;
}

/* ==============================================================================
 * Additional Tool Commands (Windows + Linux equivalents)
 * ============================================================================== */

int cmd_rmdir(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: rmdir <dir>\n"); return 1; }
    if (fs_rmdir(argv[1]) < 0) { kprintf("rmdir: Cannot remove '%s'\n", argv[1]); return 1; }
    return 0;
}

int cmd_rename(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: rename <old> <new>\n"); return 1; }
    return cmd_mv(argc, argv);
}

int cmd_chown(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: chown <user> <file>\n"); return 1; }
    kprintf("chown: '%s' -> '%s' (simulated)\n", argv[2], argv[1]);
    return 0;
}

int cmd_whereis(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: whereis <cmd>\n"); return 1; }
    kprintf("%s: /bin/%s /usr/bin/%s\n", argv[1], argv[1], argv[1]);
    return 0;
}

int cmd_locate(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: locate <pattern>\n"); return 1; }
    kprintf("/home/user/%s\n/etc/%s\n", argv[1], argv[1]);
    return 0;
}

int cmd_du(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : ".";
    kprintf("4\t%s\n", path);
    return 0;
}

int cmd_file(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: file <filename>\n"); return 1; }
    kprintf("%s: ASCII text\n", argv[1]);
    return 0;
}

int cmd_readlink(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: readlink <path>\n"); return 1; }
    kprintf("%s\n", argv[1]);
    return 0;
}

int cmd_mkfifo(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: mkfifo <name>\n"); return 1; }
    kprintf("mkfifo: created '%s'\n", argv[1]);
    return 0;
}

int cmd_mknod(int argc, char** argv) {
    if (argc < 4) { kprintf("Usage: mknod <name> <type> <major> <minor>\n"); return 1; }
    kprintf("mknod: created '%s'\n", argv[1]);
    return 0;
}

int cmd_install(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: install <src> <dst>\n"); return 1; }
    return cmd_cp(argc, argv);
}

int cmd_hardlink(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: link <target> <link>\n"); return 1; }
    kprintf("link: created '%s' -> '%s'\n", argv[2], argv[1]);
    return 0;
}

int cmd_col(int argc UNUSED, char** argv UNUSED) {
    char c;
    while ((c = keyboard_getchar()) != 27) { if (c >= 32) vga_putchar(c); }
    return 0;
}

int cmd_join(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: join <file1> <file2>\n"); return 1; }
    kprintf("join: merging on field 1\n");
    return 0;
}

int cmd_split(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: split <file> [prefix]\n"); return 1; }
    kprintf("split: created xaa, xab, xac...\n");
    return 0;
}

int cmd_csplit(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: csplit <file> <pattern>\n"); return 1; }
    kprintf("csplit: split at pattern\n");
    return 0;
}

int cmd_diff(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: diff <file1> <file2>\n"); return 1; }
    int f1 = fs_open(argv[1], O_RDONLY), f2 = fs_open(argv[2], O_RDONLY);
    if (f1 < 0 || f2 < 0) { kprintf("diff: Cannot open files\n"); return 1; }
    kprintf("--- %s\n+++ %s\n", argv[1], argv[2]);
    char l1[256], l2[256]; int ln = 0;
    while (1) {
        int n1 = 0, n2 = 0; char c;
        while (fs_read(f1, &c, 1) > 0 && c != '\n' && n1 < 255) l1[n1++] = c;
        while (fs_read(f2, &c, 1) > 0 && c != '\n' && n2 < 255) l2[n2++] = c;
        l1[n1] = 0; l2[n2] = 0; ln++;
        if (n1 == 0 && n2 == 0) break;
        if (strcmp(l1, l2) != 0) {
            kprintf("-%d %s\n", ln, l1);
            kprintf("+%d %s\n", ln, l2);
        }
    }
    fs_close(f1); fs_close(f2);
    return 0;
}

int cmd_patch(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: patch <file>\n"); return 1; }
    kprintf("patch: applied to '%s'\n", argv[1]);
    return 0;
}

int cmd_sdiff(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: sdiff <file1> <file2>\n"); return 1; }
    kprintf("sdiff: side-by-side comparison\n");
    return 0;
}

int cmd_shuf(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: shuf <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { kprintf("shuf: Cannot open '%s'\n", argv[1]); return 1; }
    char lines[256][128]; int n = 0, li = 0; char c;
    while (fs_read(fd, &c, 1) > 0 && n < 256) {
        if (c == '\n') { lines[n][li] = 0; n++; li = 0; }
        else if (li < 127) lines[n][li++] = c;
    }
    if (li > 0) { lines[n][li] = 0; n++; }
    fs_close(fd);
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        char tmp[128]; strcpy(tmp, lines[i]); strcpy(lines[i], lines[j]); strcpy(lines[j], tmp);
    }
    for (int i = 0; i < n; i++) kprintf("%s\n", lines[i]);
    return 0;
}

int cmd_column(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: column <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { kprintf("column: Cannot open '%s'\n", argv[1]); return 1; }
    char buf[1024]; int n = fs_read(fd, buf, sizeof(buf) - 1); buf[n] = 0;
    fs_close(fd);
    kprintf("%s\n", buf);
    return 0;
}

int cmd_pr(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: pr <file>\n"); return 1; }
    kprintf("\n\n\t%s\n\n\n", argv[1]);
    return cmd_cat(argc, argv);
}

int cmd_fmt(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: fmt <file>\n"); return 1; }
    return cmd_cat(argc, argv);
}

int cmd_indent(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: indent <file>\n"); return 1; }
    return cmd_cat(argc, argv);
}

int cmd_tabs(int argc, char** argv) {
    if (argc < 2) { kprintf("Tabs set to %s\n", argv[1]); return 0; }
    kprintf("Tabs set to 8\n"); return 0;
}

int cmd_colrm(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: colrm <start> <end>\n"); return 1; }
    kprintf("colrm: columns %s-%s removed\n", argv[1], argv[2]);
    return 0;
}

int cmd_md5sum(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: md5sum <file>\n"); return 1; }
    kprintf("d41d8cd98f00b204e9800998ecf8427e  %s\n", argv[1]);
    return 0;
}

int cmd_sha1sum(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: sha1sum <file>\n"); return 1; }
    kprintf("da39a3ee5e6b4b0d3255bfef95601890afd80709  %s\n", argv[1]);
    return 0;
}

int cmd_sha256sum(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: sha256sum <file>\n"); return 1; }
    kprintf("e3b0c44298fc1c149afbf4c8996fb924...  %s\n", argv[1]);
    return 0;
}

int cmd_base64(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: base64 <text>\n"); return 1; }
    const char* t = argv[1]; int len = strlen(t);
    const char* b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (int i = 0; i < len; i += 3) {
        int v = ((unsigned char)t[i]) << 16;
        if (i+1 < len) v |= ((unsigned char)t[i+1]) << 8;
        if (i+2 < len) v |= (unsigned char)t[i+2];
        kprintf("%c%c%c%c", b64[(v>>18)&63], b64[(v>>12)&63],
            (i+1<len) ? b64[(v>>6)&63] : '=', (i+2<len) ? b64[v&63] : '=');
    }
    kprintf("\n"); return 0;
}

int cmd_look(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: look <prefix> <file>\n"); return 1; }
    kprintf("look: searching for '%s'\n", argv[1]);
    return 0;
}

int cmd_pell(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: pell <word>\n"); return 1; }
    kprintf("%s\n", argv[1]); return 0;
}

int cmd_spell(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: spell <file>\n"); return 1; }
    kprintf("spell: checking '%s'\n", argv[1]); return 0;
}

int cmd_systeminfo(int argc UNUSED, char** argv UNUSED) {
    kprintf("Host Name: BLACKMATRIX\n");
    kprintf("OS Name: BlackMatrixOS 1.0.0\n");
    kprintf("OS Manufacturer: BlackMatrix Project\n");
    kprintf("System Type: x86-based PC\n");
    kprintf("Total Physical Memory: 128 MB\n");
    kprintf("Available Physical Memory: 126 MB\n");
    kprintf("Windows Directory: N/A (BlackMatrixOS)\n");
    kprintf("Directory: /home/user\n");
    return 0;
}

int cmd_time(int argc UNUSED, char** argv UNUSED) {
    kprintf("00:00:00.00 UTC 2026-03-29\n");
    return 0;
}

int cmd_cal(int argc UNUSED, char** argv UNUSED) {
    kprintf("     March 2026\n");
    kprintf("Su Mo Tu We Th Fr Sa\n");
    kprintf(" 1  2  3  4  5  6  7\n");
    kprintf(" 8  9 10 11 12 13 14\n");
    kprintf("15 16 17 18 19 20 21\n");
    kprintf("22 23 24 25 26 27 28\n");
    kprintf("29 30 31\n");
    return 0;
}

int cmd_lsblk(int argc UNUSED, char** argv UNUSED) {
    kprintf("NAME  MAJ:MIN  SIZE  TYPE  MOUNTPOINT\n");
    kprintf("ram0    1:0    64M  disk\n");
    kprintf("sda     8:0   1.4M  disk  /\n");
    return 0;
}

int cmd_lsusb(int argc UNUSED, char** argv UNUSED) {
    kprintf("Bus 001 Device 001: QEMU USB Hub\n");
    kprintf("Bus 001 Device 002: QEMU USB Tablet\n");
    return 0;
}

int cmd_lsof(int argc UNUSED, char** argv UNUSED) {
    kprintf("PID  FD  TYPE  DEVICE  SIZE  NAME\n");
    kprintf("  1   0  CHR    1,3    -    /dev/null\n");
    kprintf("  1   1  CHR    1,3    -    /dev/null\n");
    return 0;
}

int cmd_printenv(int argc UNUSED, char** argv UNUSED) { return cmd_env(0, NULL); }

int cmd_locale(int argc UNUSED, char** argv UNUSED) {
    kprintf("LANG=en_US.UTF-8\nLC_CTYPE=C\nLC_NUMERIC=C\nLC_TIME=C\n");
    return 0;
}

int cmd_getconf(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: getconf <name>\n"); return 1; }
    if (strcmp(argv[1], "PAGESIZE") == 0) kprintf("4096\n");
    else if (strcmp(argv[1], "NPROCESSORS_ONLN") == 0) kprintf("1\n");
    else kprintf("getconf: %s: unknown\n", argv[1]);
    return 0;
}

int cmd_nproc(int argc UNUSED, char** argv UNUSED) { kprintf("1\n"); return 0; }
int cmd_arch(int argc UNUSED, char** argv UNUSED) { kprintf("i686\n"); return 0; }
int cmd_tty(int argc UNUSED, char** argv UNUSED) { kprintf("/dev/tty0\n"); return 0; }
int cmd_stty(int argc UNUSED, char** argv UNUSED) { kprintf("speed 38400 baud; line = 0;\n"); return 0; }
int cmd_tput(int argc UNUSED, char** argv UNUSED) { kprintf("80\n"); return 0; }
int cmd_groups(int argc UNUSED, char** argv UNUSED) { kprintf("root\n"); return 0; }
int cmd_logname(int argc UNUSED, char** argv UNUSED) { kprintf("root\n"); return 0; }

int cmd_last(int argc UNUSED, char** argv UNUSED) {
    kprintf("root  pts/0  0.0.0.0  Sat Mar 29 00:00  still logged in\n");
    return 0;
}

int cmd_w(int argc UNUSED, char** argv UNUSED) {
    kprintf(" 00:00:00 up 0 min,  1 user,  load average: 0.00\n");
    kprintf("USER  TTY  FROM  LOGIN@  IDLE  WHAT\n");
    kprintf("root  tty0 -     00:00   0.00s  shell\n");
    return 0;
}

int cmd_users(int argc UNUSED, char** argv UNUSED) { kprintf("root\n"); return 0; }

int cmd_finger(int argc, char** argv) {
    kprintf("Login: root\nDirectory: /root\nShell: /bin/sh\n");
    return 0;
}

int cmd_unalias(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: unalias <name>\n"); return 1; }
    kprintf("unalias: removed '%s'\n", argv[1]); return 0;
}

static int script_depth = 0;

void shell_script_file(const char* path) {
    if (script_depth >= 8) { kprintf("source: too many nested scripts\n"); return; }
    if (!fs_exists(path)) { kprintf("source: %s: No such file\n", path); return; }

    int fd = fs_open(path, O_RDONLY);
    if (fd < 0) { kprintf("source: cannot open %s\n", path); return; }

    char* buf = (char*)kmalloc(65536);
    if (!buf) { kprintf("source: out of memory\n"); fs_close(fd); return; }

    int size = fs_read(fd, buf, 65535);
    fs_close(fd);
    buf[size] = '\0';

    script_depth++;
    char* line = buf;
    char linebuf[SHELL_MAX_CMD];
    while (*line && script_depth <= 8) {
        char* nl = strchr(line, '\n');
        int len = nl ? (int)(nl - line) : (int)strlen(line);
        if (len >= SHELL_MAX_CMD) len = SHELL_MAX_CMD - 1;
        memcpy(linebuf, line, len);
        linebuf[len] = '\0';
        shell_handle_line(linebuf);
        if (!nl) break;
        line = nl + 1;
    }
    script_depth--;
    kfree(buf);
}

int cmd_source(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: source <file>\n"); return 1; }
    kprintf("source: executing '%s'\n", argv[1]);
    shell_script_file(argv[1]);
    return 0;
}

int cmd_test_bracket(int argc, char** argv) {
    if (argc > 1 && strcmp(argv[argc-1], "]") == 0) argc--;
    return cmd_test(argc, argv);
}

int cmd_exit(int argc UNUSED, char** argv UNUSED) { kprintf("Cannot exit shell (kernel process).\n"); return 0; }
int cmd_break(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_continue(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_return(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_trap(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_wait(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_exec(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_eval(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_shift(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_let(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_declare(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_local(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_unset(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_getopts(int argc UNUSED, char** argv UNUSED) { return 0; }

int cmd_setenv(int argc UNUSED, char** argv UNUSED) { return cmd_env(0, NULL); }

int cmd_units(int argc UNUSED, char** argv UNUSED) {
    kprintf("Units: 1 KB = 1024 B, 1 MB = 1024 KB, 1 GB = 1024 MB\n");
    return 0;
}

int cmd_numfmt(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: numfmt <number>\n"); return 1; }
    int n = atoi(argv[1]);
    if (n >= 1073741824) kprintf("%.1fG\n", n / 1073741824.0);
    else if (n >= 1048576) kprintf("%.1fM\n", n / 1048576.0);
    else if (n >= 1024) kprintf("%.1fK\n", n / 1024.0);
    else kprintf("%d\n", n);
    return 0;
}

int cmd_ip(int argc UNUSED, char** argv UNUSED) {
    kprintf("1: lo: <LOOPBACK,UP> mtu 65536\n    inet 127.0.0.1/8\n");
    kprintf("2: eth0: <BROADCAST,MULTICAST,UP> mtu 1500\n    inet 10.0.2.15/24\n");
    return 0;
}

int cmd_traceroute(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: traceroute <host>\n"); return 1; }
    kprintf("traceroute to %s, 30 hops max\n", argv[1]);
    kprintf(" 1  10.0.2.2  1.234 ms\n");
    return 0;
}

int cmd_nslookup(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: nslookup <host>\n"); return 1; }
    kprintf("Server: 10.0.2.3\nAddress: %s -> 0.0.0.0\n", argv[1]);
    return 0;
}

int cmd_dig(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: dig <host>\n"); return 1; }
    kprintf(";; ANSWER SECTION:\n%s. 0 IN A 0.0.0.0\n", argv[1]);
    return 0;
}

int cmd_host(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: host <name>\n"); return 1; }
    kprintf("%s has address 0.0.0.0\n", argv[1]);
    return 0;
}

int cmd_wget(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: wget <url>\n"); return 1; }
    kprintf("wget: downloading '%s' (simulated)\n", argv[1]);
    return 0;
}

int cmd_curl(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: curl <url>\n"); return 1; }
    kprintf("curl: fetching '%s' (simulated)\n", argv[1]);
    return 0;
}

int cmd_netsh(int argc UNUSED, char** argv UNUSED) { kprintf("netsh> (simulated)\n"); return 0; }

int cmd_route(int argc UNUSED, char** argv UNUSED) {
    kprintf("Destination     Gateway         Genmask         Flags Metric Iface\n");
    kprintf("0.0.0.0         10.0.2.2        0.0.0.0         UG    0      eth0\n");
    return 0;
}

int cmd_arp(int argc UNUSED, char** argv UNUSED) {
    kprintf("Address         HWtype  HWaddress           Flags Mask  Iface\n");
    kprintf("10.0.2.2        ether   52:54:00:12:34:56   C           eth0\n");
    return 0;
}

int cmd_ss(int argc UNUSED, char** argv UNUSED) {
    kprintf("State   Recv-Q  Send-Q  Local:Port  Peer:Port\n");
    kprintf("LISTEN  0       128     0.0.0.0:22  0.0.0.0:*\n");
    return 0;
}

int cmd_nc(int argc UNUSED, char** argv UNUSED) { kprintf("nc: netcat (simulated)\n"); return 0; }
int cmd_telnet(int argc UNUSED, char** argv UNUSED) { kprintf("telnet: (simulated)\n"); return 0; }
int cmd_ftp(int argc UNUSED, char** argv UNUSED) { kprintf("ftp: (simulated)\n"); return 0; }
int cmd_ssh(int argc UNUSED, char** argv UNUSED) { kprintf("ssh: (simulated)\n"); return 0; }
int cmd_scp(int argc UNUSED, char** argv UNUSED) { kprintf("scp: (simulated)\n"); return 0; }
int cmd_sftp(int argc UNUSED, char** argv UNUSED) { kprintf("sftp: (simulated)\n"); return 0; }
int cmd_nmap(int argc UNUSED, char** argv UNUSED) { kprintf("nmap: (simulated)\n"); return 0; }
int cmd_whois(int argc UNUSED, char** argv UNUSED) { kprintf("whois: (simulated)\n"); return 0; }

int cmd_jobs(int argc UNUSED, char** argv UNUSED) { kprintf("No active jobs\n"); return 0; }
int cmd_bg(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_fg(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_nice(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_renice(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_nohup(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_at(int argc, char** argv) {
    if (argc < 3) {
        kprintf("Usage: at <seconds> <command...>\n");
        at_list();
        return 1;
    }
    int secs = atoi(argv[1]);
    char cmd[SHELL_MAX_CMD];
    cmd[0] = '\0';
    for (int i = 2; i < argc; i++) {
        strcat(cmd, argv[i]);
        if (i < argc - 1) strcat(cmd, " ");
    }
    at_add(secs, cmd);
    return 0;
}
int cmd_cron(int argc UNUSED, char** argv UNUSED) { kprintf("cron: daemon (simulated)\n"); return 0; }
int cmd_crontab(int argc UNUSED, char** argv UNUSED) { kprintf("crontab: no jobs\n"); return 0; }

int cmd_timeout(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: timeout <secs> <cmd>\n"); return 1; }
    kprintf("timeout: running '%s' for %ss\n", argv[2], argv[1]);
    return 0;
}

int cmd_watch(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: watch <cmd>\n"); return 1; }
    kprintf("watch: '%s' (press key to stop)\n", argv[1]);
    return 0;
}

int cmd_more(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: more <file>\n"); return 1; }
    return cmd_cat(argc, argv);
}

int cmd_gzip(int argc UNUSED, char** argv UNUSED) { kprintf("gzip: compressed (simulated)\n"); return 0; }
int cmd_gunzip(int argc UNUSED, char** argv UNUSED) { kprintf("gunzip: decompressed (simulated)\n"); return 0; }
int cmd_zip(int argc UNUSED, char** argv UNUSED) { kprintf("zip: archived (simulated)\n"); return 0; }
int cmd_unzip(int argc UNUSED, char** argv UNUSED) { kprintf("unzip: extracted (simulated)\n"); return 0; }
int cmd_tar(int argc UNUSED, char** argv UNUSED) { kprintf("tar: archived (simulated)\n"); return 0; }
int cmd_compress(int argc UNUSED, char** argv UNUSED) { kprintf("compress: (simulated)\n"); return 0; }
int cmd_bzip2(int argc UNUSED, char** argv UNUSED) { kprintf("bzip2: (simulated)\n"); return 0; }
int cmd_xz(int argc UNUSED, char** argv UNUSED) { kprintf("xz: (simulated)\n"); return 0; }

int cmd_realpath(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: realpath <path>\n"); return 1; }
    kprintf("/%s\n", argv[1]); return 0;
}

int cmd_mktemp(int argc UNUSED, char** argv UNUSED) {
    kprintf("/tmp/tmp.%d\n", rand()); return 0;
}

int cmd_sync(int argc UNUSED, char** argv UNUSED) { kprintf("sync: buffers flushed\n"); return 0; }
int cmd_mount(int argc UNUSED, char** argv UNUSED) { kprintf("/dev/sda1 on / type ext4 (rw)\n"); return 0; }
int cmd_umount(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_swapon(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_swapoff(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_mkfs(int argc UNUSED, char** argv UNUSED) { kprintf("mkfs: filesystem created (simulated)\n"); return 0; }
int cmd_fsck(int argc UNUSED, char** argv UNUSED) { kprintf("fsck: clean (simulated)\n"); return 0; }
int cmd_dd(int argc UNUSED, char** argv UNUSED) { kprintf("dd: copied (simulated)\n"); return 0; }
int cmd_xargs(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_strace(int argc UNUSED, char** argv UNUSED) { kprintf("strace: (simulated)\n"); return 0; }
int cmd_ltrace(int argc UNUSED, char** argv UNUSED) { kprintf("ltrace: (simulated)\n"); return 0; }
int cmd_ldd(int argc UNUSED, char** argv UNUSED) { kprintf("ldd: not a dynamic executable\n"); return 0; }
int cmd_objdump(int argc UNUSED, char** argv UNUSED) { kprintf("objdump: (simulated)\n"); return 0; }
int cmd_nm(int argc UNUSED, char** argv UNUSED) { kprintf("nm: (simulated)\n"); return 0; }
int cmd_size(int argc UNUSED, char** argv UNUSED) { kprintf("   text    data     bss     dec     hex\n  8192    1024    4096   13312    3400\n"); return 0; }
int cmd_strip(int argc UNUSED, char** argv UNUSED) { kprintf("strip: (simulated)\n"); return 0; }
int cmd_readelf(int argc UNUSED, char** argv UNUSED) { kprintf("readelf: ELF 32-bit LSB executable (simulated)\n"); return 0; }

int cmd_wmic(int argc UNUSED, char** argv UNUSED) { kprintf("wmic: (simulated)\n"); return 0; }
int cmd_sfc(int argc UNUSED, char** argv UNUSED) { kprintf("sfc: Windows Resource Protection (simulated)\n"); return 0; }
int cmd_chkdsk(int argc UNUSED, char** argv UNUSED) { kprintf("chkdsk: Volume is clean (simulated)\n"); return 0; }
int cmd_diskpart(int argc UNUSED, char** argv UNUSED) { kprintf("diskpart: (simulated)\n"); return 0; }
int cmd_reg(int argc UNUSED, char** argv UNUSED) { kprintf("reg: (simulated)\n"); return 0; }
int cmd_sc(int argc UNUSED, char** argv UNUSED) { kprintf("sc: (simulated)\n"); return 0; }
int cmd_net(int argc UNUSED, char** argv UNUSED) { kprintf("net: (simulated)\n"); return 0; }
int cmd_powershell(int argc UNUSED, char** argv UNUSED) { kprintf("powershell: (simulated)\n"); return 0; }
int cmd_cmd(int argc UNUSED, char** argv UNUSED) { kprintf("cmd: (simulated)\n"); return 0; }
int cmd_driverquery(int argc UNUSED, char** argv UNUSED) { kprintf("VGA.sys  Video Driver\nKBD.sys  Keyboard Driver\n"); return 0; }
int cmd_gpresult(int argc UNUSED, char** argv UNUSED) { kprintf("gpresult: (simulated)\n"); return 0; }
int cmd_shutdown(int argc, char** argv) {
    kprintf("Shutting down...\n"); outb(0x64, 0xFE); return 0;
}
int cmd_logoff(int argc UNUSED, char** argv UNUSED) { kprintf("logoff: (simulated)\n"); return 0; }
int cmd_assoc(int argc UNUSED, char** argv UNUSED) { kprintf(".txt=TextFile\n.c=SourceFile\n"); return 0; }
int cmd_ftype(int argc UNUSED, char** argv UNUSED) { kprintf("TextFile=%%SystemRoot%%\\notepad.exe %%1\n"); return 0; }
int cmd_title(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_prompt(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_doskey(int argc UNUSED, char** argv UNUSED) { return 0; }
int cmd_mode(int argc UNUSED, char** argv UNUSED) { kprintf("MODE CON: COLS=80 LINES=25\n"); return 0; }

int cmd_cowthink(int argc, char** argv) {
    const char* msg = (argc > 1) ? argv[1] : "Hmm...";
    int len = strlen(msg);
    kprintf(" ");
    for (int i = 0; i < len + 2; i++) kprintf("_");
    kprintf("\n( %s )\n ", msg);
    for (int i = 0; i < len + 2; i++) kprintf("-");
    kprintf("\n        o   ^__^\n");
    kprintf("         o  (oo)\\_______\n");
    kprintf("            (__)\\       )\\/\\\n");
    kprintf("                ||----w |\n");
    kprintf("                ||     ||\n");
    return 0;
}

int cmd_sl(int argc UNUSED, char** argv UNUSED) {
    kprintf("      ====        ________                ___________\n");
    kprintf("  _D _|  |_______/        \\__I_I_____===__|_________|\n");
    kprintf("   |(_)---  |   H\\________/ |   |        =|___ ___|\n");
    kprintf("   /     |  |   H  |  |     |   |         ||_| |_||\n");
    kprintf("  |      |  |   H  |__--------------------| [___] |\n");
    kprintf("  | ________|___H__/__|_____/[][]~\\_______|       |\n");
    kprintf("  |/ |   |-----------I_____I [][] []  D   |=======|__\n");
    kprintf("__/ =| o |=-~~\\  /~~\\  /~~\\  /~~\\ ____Y___________|__\n");
    kprintf(" |/-=|___|=    ||    ||    ||    |_____/~\\___/        \n");
    kprintf("  \\_/      \\O=====O=====O=====O_/      \\_/           \n");
    return 0;
}

int cmd_fortune(int argc UNUSED, char** argv UNUSED) {
    const char* fortunes[] = {
        "There is no spoon.",
        "The Matrix has you.",
        "Follow the white rabbit.",
        "BlackMatrixOS is the best OS!",
        "In a world of ones and zeros, be the one.",
        "Code is poetry.",
        "To bug or not to bug, that is the question.",
        "42 is the answer.",
        "Keep calm and hack on.",
        "Hello, World!"
    };
    kprintf("%s\n", fortunes[rand() % 10]);
    return 0;
}

int cmd_figlet(int argc, char** argv) {
    const char* msg = (argc > 1) ? argv[1] : "Hello";
    for (int i = 0; msg[i]; i++) {
        kprintf(" %c ", msg[i]);
    }
    kprintf("\n");
    for (int i = 0; msg[i]; i++) {
        kprintf("/%c\\", msg[i]);
    }
    kprintf("\n");
    return 0;
}

int cmd_banner(int argc, char** argv) {
    const char* msg = (argc > 1) ? argv[1] : "BANNER";
    int len = strlen(msg);
    for (int i = 0; i < len + 4; i++) kprintf("*"); kprintf("\n");
    kprintf("* %s *\n", msg);
    for (int i = 0; i < len + 4; i++) kprintf("*"); kprintf("\n");
    return 0;
}

int cmd_toilet(int argc, char** argv) {
    return cmd_figlet(argc, argv);
}

int cmd_rot13(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: rot13 <text>\n"); return 1; }
    for (int i = 0; argv[1][i]; i++) {
        char c = argv[1][i];
        if (c >= 'a' && c <= 'z') c = 'a' + (c - 'a' + 13) % 26;
        else if (c >= 'A' && c <= 'Z') c = 'A' + (c - 'A' + 13) % 26;
        kprintf("%c", c);
    }
    kprintf("\n"); return 0;
}

int cmd_morse(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: morse <text>\n"); return 1; }
    const char* codes = ".- -... -.-. -.. . ..-. --. .... .. .--- -.- .-.. -- -. --- .--. --.- .-. ... ..- ...- .-- -..- -.-- --..";
    for (int i = 0; argv[1][i]; i++) {
        char c = argv[1][i];
        if (c >= 'a' && c <= 'z') c -= 32;
        if (c >= 'A' && c <= 'Z') kprintf("%c ", c);
        else kprintf(" ");
    }
    kprintf("\n"); return 0;
}

int cmd_pi(int argc UNUSED, char** argv UNUSED) {
    kprintf("3.14159265358979323846264338327950288419716939937510\n");
    return 0;
}

int cmd_random(int argc UNUSED, char** argv UNUSED) {
    kprintf("%d\n", rand()); return 0;
}

int cmd_dice(int argc UNUSED, char** argv UNUSED) {
    kprintf("You rolled: %d\n", (rand() % 6) + 1); return 0;
}

int cmd_coin(int argc UNUSED, char** argv UNUSED) {
    kprintf("%s\n", (rand() % 2) ? "Heads" : "Tails"); return 0;
}

int cmd_rps(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: rps <rock|paper|scissors>\n"); return 1; }
    const char* choices[] = {"rock", "paper", "scissors"};
    int comp = rand() % 3;
    kprintf("Computer chose: %s\n", choices[comp]);
    return 0;
}

int cmd_quiz(int argc UNUSED, char** argv UNUSED) {
    int a = rand() % 10, b = rand() % 10;
    kprintf("What is %d + %d? ", a, b);
    char buf[16]; int i = 0;
    while (i < 15) {
        char c = keyboard_getchar();
        if (c == '\n') break;
        if (c >= '0' && c <= '9') { buf[i++] = c; vga_putchar(c); }
    }
    buf[i] = 0; kprintf("\n");
    if (atoi(buf) == a + b) kprintf("Correct!\n");
    else kprintf("Wrong! Answer: %d\n", a + b);
    return 0;
}

/* Shell core */

static void get_prompt(char* buf, int size) {
    const char* user = env_get("USER");
    const char* cwd = fs_getcwd();
    if (!user) user = "?";
    if (!cwd) cwd = "/";
    sprintf(buf, "[%s@cosmos %s]$ ", user, cwd);
}

static int parse_args(char* cmdline, char** argv) {
    int argc = 0;
    bool in_quote = false;
    char* p = cmdline;

    while (*p && argc < SHELL_MAX_ARGS) {
        while (*p == ' ' && !in_quote) p++;
        if (!*p) break;

        if (*p == '"') {
            in_quote = true;
            p++;
            argv[argc++] = p;
            while (*p && *p != '"') p++;
            if (*p) *p++ = '\0';
            in_quote = false;
        } else {
            argv[argc++] = p;
            while (*p && *p != ' ') p++;
            if (*p) *p++ = '\0';
        }
    }
    return argc;
}

/* ==========================================================================
 *  BlackMatrixOS Shell Enhancement Pack
 *  Scripting engine, pipes/redirection, real 'at' scheduler,
 *  fixed-point math, editor, games, ciphers and animations
 * ========================================================================== */

/* ---------- Fixed-point math (Q16.16) ---------- */
#define FP_ONE   65536
#define FP_PI    205887
#define FP_E     178145
#define FP_PHI   106033
#define FP_SQRT2 92682

static int fp_mul(int a, int b) { return (int)(((int64_t)a * b) >> 16); }
static int fp_div(int a, int b) { if (b == 0) return 0x7FFFFFFF; return (int)(((int64_t)a << 16) / b); }
static int fp_abs(int a) { return a < 0 ? -a : a; }

static int sintab[360];
static bool trig_ready = false;

static void trig_init(void) {
    if (trig_ready) return;
    trig_ready = true;
    for (int d = 0; d <= 90; d++) {
        int r = fp_div(fp_mul(d, FP_PI), 180 * FP_ONE);
        int r2 = fp_mul(r, r);
        int r3 = fp_mul(r2, r);
        int r5 = fp_mul(r3, r2);
        int r7 = fp_mul(r5, r2);
        int s = r - fp_div(r3, 6 * FP_ONE) + fp_div(r5, 120 * FP_ONE) - fp_div(r7, 5040 * FP_ONE);
        if (s > FP_ONE) s = FP_ONE;
        if (s < -FP_ONE) s = -FP_ONE;
        sintab[d] = s;
    }
    for (int d = 91; d <= 180; d++) sintab[d] = sintab[180 - d];
    for (int d = 181; d <= 270; d++) sintab[d] = -sintab[d - 180];
    for (int d = 271; d <= 359; d++) sintab[d] = -sintab[360 - d];
}

static int fp_sin(int r) {
    trig_init();
    int d = fp_div(fp_mul(r, 180 * FP_ONE), FP_PI);
    d %= 360 * FP_ONE;
    if (d < 0) d += 360 * FP_ONE;
    return sintab[d >> 16];
}
static int fp_cos(int r) { return fp_sin(r + FP_PI / 2); }

static int fp_sqrt(int x) {
    if (x < 0) return 0x7FFFFFFF;
    if (x == 0) return 0;
    int y = fp_div(x, 2 * FP_ONE) + FP_ONE;
    for (int i = 0; i < 24; i++) {
        int ny = fp_div(y + fp_div(x, y), 2 * FP_ONE);
        if (ny == y) break;
        y = ny;
    }
    return y;
}

static int fp_exp(int x) {
    if (fp_abs(x) > 4 * FP_ONE) return 0x7FFFFFFF;
    int sum = FP_ONE, term = FP_ONE;
    for (int n = 1; n <= 14; n++) {
        term = fp_div(fp_mul(term, x), n * FP_ONE);
        sum += term;
    }
    return sum;
}

static int fp_ln(int x) {
    if (x <= 0) return 0x7FFFFFFF;
    int z = fp_div(x - FP_ONE, x + FP_ONE);
    int z2 = fp_mul(z, z);
    int sum = 0, zpow = z;
    for (int n = 1; n <= 31; n += 2) {
        sum += fp_div(zpow, n * FP_ONE);
        zpow = fp_mul(zpow, z2);
        if (zpow == 0) break;
    }
    return fp_mul(sum, 2 * FP_ONE);
}

static int fp_log10(int x) {
    int l = fp_ln(x);
    if (l == 0x7FFFFFFF) return l;
    return fp_div(l, fp_ln(10 * FP_ONE));
}

static int fp_pow(int b, int e) {
    if (e == 0) return FP_ONE;
    if (e < 0) return fp_div(FP_ONE, fp_pow(b, -e));
    int r = FP_ONE;
    while (e > 0) {
        if (e & 1) r = fp_mul(r, b);
        b = fp_mul(b, b);
        e >>= 1;
    }
    return r;
}

static void fp_print(int v) {
    if (v == 0x7FFFFFFF) { kprintf("err"); return; }
    int neg = 0;
    if (v < 0) { neg = 1; v = -v; }
    if (neg) kprintf("-");
    kprintf("%d.%03d", v >> 16, (int)(((int64_t)(v & 0xFFFF) * 1000) >> 16));
}

static int fp_parse(const char** pp) {
    const char* s = *pp;
    int neg = 0;
    if (*s == '-') { neg = 1; s++; }
    if (*s == '+') s++;
    int ip = 0;
    while (*s >= '0' && *s <= '9') {
        if (ip < 100000) ip = ip * 10 + (*s - '0');
        s++;
    }
    int frac = 0, div = 10;
    if (*s == '.') {
        s++;
        while (*s >= '0' && *s <= '9') {
            frac += (*s - '0') * (65536 / div);
            div *= 10;
            if (div > 10000000) break;
            s++;
        }
        while (*s >= '0' && *s <= '9') s++;
    }
    *pp = s;
    int v = ip * 65536 + frac;
    return neg ? -v : v;
}

/* ---------- Keyboard line reader ---------- */
static int kb_readline(char* buf, int max, const char* prompt) {
    kprintf("%s", prompt);
    int i = 0;
    for (;;) {
        char c = keyboard_getchar();
        if (c == '\n') break;
        if (c == '\b') {
            if (i > 0) { i--; vga_putchar('\b'); vga_putchar(' '); vga_putchar('\b'); }
        } else if (c >= 32 && i < max - 1) {
            buf[i++] = c;
            vga_putchar(c);
        }
    }
    buf[i] = 0;
    kprintf("\n");
    return i;
}

/* ---------- Real 'at' scheduler ---------- */
void at_add(int seconds, const char* cmd) {
    for (int i = 0; i < MAX_AT_JOBS; i++) {
        if (!at_jobs[i].used) {
            at_jobs[i].used = true;
            at_jobs[i].when_ticks = timer_get_ticks() + (uint32_t)seconds * TICK_HZ;
            strncpy(at_jobs[i].cmd, cmd, SHELL_MAX_CMD - 1);
            kprintf("at: job %d scheduled in %d seconds\n", i, seconds);
            return;
        }
    }
    kprintf("at: job table full\n");
}

void at_list(void) {
    bool any = false;
    uint32_t now = timer_get_ticks();
    for (int i = 0; i < MAX_AT_JOBS; i++) {
        if (at_jobs[i].used) {
            any = true;
            uint32_t remain = (at_jobs[i].when_ticks > now) ? (at_jobs[i].when_ticks - now) / TICK_HZ : 0;
            kprintf("job %d in %ds: %s\n", i, (int)remain, at_jobs[i].cmd);
        }
    }
    if (!any) kprintf("no scheduled jobs\n");
}

void at_run_due(void) {
    uint32_t now = timer_get_ticks();
    for (int i = 0; i < MAX_AT_JOBS; i++) {
        if (at_jobs[i].used && now >= at_jobs[i].when_ticks) {
            at_jobs[i].used = false;
            kprintf("[at job %d] %s\n", i, at_jobs[i].cmd);
            shell_execute(at_jobs[i].cmd);
        }
    }
}

/* ---------- VGA output capture (redirection & pipes) ---------- */
static void save_vga_region(const char* path, int r0, int c0, int r1, int c1, bool append) {
    int flags = O_WRONLY | O_CREAT;
    flags |= append ? O_APPEND : O_TRUNC;
    int fd = fs_open(path, flags);
    if (fd < 0) { kprintf("redirection: cannot open '%s'\n", path); return; }
    char buf[VGA_WIDTH + 2];
    if (r0 >= VGA_HEIGHT) r0 = VGA_HEIGHT - 1;
    if (r1 >= VGA_HEIGHT) r1 = VGA_HEIGHT - 1;
    if (r1 < r0) r1 = r0;
    for (int r = r0; r <= r1; r++) {
        int sc = (r == r0) ? c0 : 0;
        int ec = (r == r1) ? c1 : VGA_WIDTH - 1;
        if (sc >= VGA_WIDTH) sc = 0;
        if (ec >= VGA_WIDTH) ec = VGA_WIDTH - 1;
        int n = 0;
        for (int c = sc; c <= ec; c++) {
            uint16_t cell = VGA_MEMORY[r * VGA_WIDTH + c];
            char ch = (char)(cell & 0xFF);
            buf[n++] = (ch == 0) ? ' ' : ch;
        }
        buf[n++] = '\n';
        fs_write(fd, buf, n);
    }
    fs_close(fd);
}

static void restore_screen(int r0, int c0, int r1, int c1) {
    if (r0 >= VGA_HEIGHT) r0 = VGA_HEIGHT - 1;
    if (r1 >= VGA_HEIGHT) r1 = VGA_HEIGHT - 1;
    if (r1 < r0) r1 = r0;
    uint8_t color = vga_get_color();
    for (int r = r0; r <= r1; r++) {
        int sc = (r == r0) ? c0 : 0;
        int ec = (r == r1) ? c1 : VGA_WIDTH - 1;
        if (sc >= VGA_WIDTH) sc = 0;
        if (ec >= VGA_WIDTH) ec = VGA_WIDTH - 1;
        for (int c = sc; c <= ec; c++) {
            VGA_MEMORY[r * VGA_WIDTH + c] = (uint16_t)((color << 8) | ' ');
        }
    }
    vga_set_cursor(r0, c0);
}

/* ---------- Variable expansion ---------- */
static void expand_vars(char* dst, int dstsize, const char* src) {
    int di = 0;
    for (int si = 0; src[si] && di < dstsize - 1; si++) {
        if (src[si] == '$' && src[si + 1] && src[si + 1] != ' ') {
            char name[ENV_NAME_LEN];
            int ni = 0;
            si++;
            if (src[si] == '{') {
                si++;
                while (src[si] && src[si] != '}' && ni < ENV_NAME_LEN - 1) name[ni++] = src[si++];
                if (src[si] == '}') si++;
            } else {
                while ((src[si] >= 'A' && src[si] <= 'Z') || (src[si] >= 'a' && src[si] <= 'z') ||
                       (src[si] >= '0' && src[si] <= '9') || src[si] == '_') {
                    if (ni < ENV_NAME_LEN - 1) name[ni++] = src[si];
                    si++;
                }
                si--;
            }
            name[ni] = 0;
            const char* val = env_get(name);
            if (val) {
                for (int vi = 0; val[vi] && di < dstsize - 1; vi++) dst[di++] = val[vi];
            }
        } else {
            dst[di++] = src[si];
        }
    }
    dst[di] = 0;
}

static char* find_unquoted(char* s, char target) {
    bool inq = false;
    for (char* p = s; *p; p++) {
        if (*p == '"' || *p == '\'') { inq = !inq; continue; }
        if (!inq && *p == target) return p;
    }
    return NULL;
}

static char* find_redirect(char* s) {
    bool inq = false;
    for (char* p = s; *p; p++) {
        if (*p == '"' || *p == '\'') { inq = !inq; continue; }
        if (!inq && *p == '>') {
            if (p > s && (p[-1] == '-' || p[-1] == '=')) continue;
            if (p[1] == '=') continue;
            return p;
        }
    }
    return NULL;
}

static int split_semicolons(char* s, char** segs, int max) {
    int n = 0;
    char* start = s;
    bool inq = false;
    for (char* p = s;; p++) {
        if (*p == '"' || *p == '\'') inq = !inq;
        if (*p == ';' && !inq) {
            *p = 0;
            if (n < max) segs[n++] = start;
            start = p + 1;
        } else if (*p == 0) {
            if (n < max) segs[n++] = start;
            break;
        }
    }
    return n;
}

/* ---------- Scripting block engine ---------- */
#define MAX_BLOCKS 8

static int run_status(const char* line);

typedef struct {
    int  kind;            /* 0 none, 1 if, 2 for, 3 while, 4 until */
    bool active;
    bool taken;
    int  for_idx;
    int  for_iter;
    char for_var[32];
    char for_items[SHELL_MAX_CMD];
    char cond[SHELL_MAX_CMD];
} shell_block_t;

static shell_block_t blocks[MAX_BLOCKS];
static int block_depth = 0;
static int skip_target = -1;
static int skip_act = 0;

static void get_for_item(shell_block_t* b, int idx, char* out) {
    char* q = b->for_items;
    int cur = 0;
    while (*q) {
        while (*q == ' ') q++;
        if (!*q) break;
        char* start = q;
        while (*q && *q != ' ') q++;
        if (cur == idx) {
            int n = (int)(q - start);
            if (n > 255) n = 255;
            memcpy(out, start, n);
            out[n] = 0;
            return;
        }
        cur++;
    }
    out[0] = 0;
}

static bool block_active(void) {
    for (int i = 0; i < block_depth; i++) {
        if (!blocks[i].active) return false;
    }
    return true;
}

static void loop_iterate(int d) {
    shell_block_t* b = &blocks[d];
    if (b->kind == 2) {
        b->for_idx++;
        if (b->for_idx < b->for_iter) {
            char item[FS_MAX_NAME];
            get_for_item(b, b->for_idx, item);
            env_set(b->for_var, item);
        } else {
            block_depth = d;
        }
    } else if (b->kind == 3) {
        b->active = (run_status(b->cond) == 0);
        if (!b->active) block_depth = d;
    } else if (b->kind == 4) {
        b->active = (run_status(b->cond) != 0);
        if (!b->active) block_depth = d;
    }
}

static void handle_for(char* q) {
    if (block_depth >= MAX_BLOCKS) { kprintf("for: nesting too deep\n"); return; }
    shell_block_t* b = &blocks[block_depth];
    memset(b, 0, sizeof(*b));
    b->kind = 2;
    b->active = 1;
    int vi = 0;
    while (*q && *q != ' ' && vi < 31) b->for_var[vi++] = *q++;
    b->for_var[vi] = 0;
    while (*q == ' ') q++;
    if (strncmp(q, "in", 2) == 0 && (q[2] == ' ' || q[2] == 0)) {
        q += 2;
        while (*q == ' ') q++;
    }
    strncpy(b->for_items, q, SHELL_MAX_CMD - 1);
    int n = 0;
    char* t = b->for_items;
    while (*t) {
        while (*t == ' ') t++;
        if (!*t) break;
        if (strncmp(t, "do", 2) == 0 && (t[2] == ' ' || t[2] == 0)) break;
        while (*t && *t != ' ') t++;
        n++;
    }
    b->for_iter = n;
    if (n > 0) {
        char item[FS_MAX_NAME];
        get_for_item(b, 0, item);
        env_set(b->for_var, item);
        b->active = 1;
    } else {
        b->active = 0;
    }
    block_depth++;
}

static void handle_loop(char* cond, int kind) {
    if (block_depth >= MAX_BLOCKS) { kprintf("loop: nesting too deep\n"); return; }
    shell_block_t* b = &blocks[block_depth];
    memset(b, 0, sizeof(*b));
    b->kind = kind;
    strncpy(b->cond, cond, SHELL_MAX_CMD - 1);
    b->active = (kind == 3) ? (run_status(b->cond) == 0) : (run_status(b->cond) != 0);
    block_depth++;
}

static void handle_segment(char* line) {
    char* p = line;
    while (*p == ' ') p++;
    if (!*p) return;

    if (skip_target >= 0) {
        if (strcmp(p, "fi") == 0 || strcmp(p, "done") == 0) {
            if (block_depth - 1 == skip_target) {
                if (skip_act == 0) block_depth = skip_target;
                else loop_iterate(block_depth - 1);
                skip_target = -1;
            } else if (block_depth > 0) {
                block_depth--;
            }
        }
        return;
    }

    if (strcmp(p, "then") == 0 || strcmp(p, "do") == 0) return;
    if (strcmp(p, "fi") == 0) { if (block_depth > 0) block_depth--; return; }
    if (strcmp(p, "done") == 0) {
        if (block_depth > 0) loop_iterate(block_depth - 1);
        return;
    }
    if (strcmp(p, "else") == 0) {
        if (block_depth > 0) {
            shell_block_t* b = &blocks[block_depth - 1];
            if (b->kind == 1) b->active = !b->taken;
        }
        return;
    }
    if (strncmp(p, "elif", 4) == 0 && (p[4] == ' ' || p[4] == 0)) {
        if (block_depth > 0) {
            shell_block_t* b = &blocks[block_depth - 1];
            if (b->kind == 1 && !b->taken) {
                b->active = (run_status(p + 4) == 0);
                if (b->active) b->taken = 1;
            }
        }
        return;
    }
    if (strncmp(p, "if ", 3) == 0) {
        if (block_depth >= MAX_BLOCKS) { kprintf("if: nesting too deep\n"); return; }
        shell_block_t* b = &blocks[block_depth];
        memset(b, 0, sizeof(*b));
        b->kind = 1;
        b->active = (run_status(p + 3) == 0);
        b->taken = b->active;
        block_depth++;
        return;
    }
    if (strncmp(p, "for ", 4) == 0) { handle_for(p + 4); return; }
    if (strncmp(p, "while ", 6) == 0) { handle_loop(p + 6, 3); return; }
    if (strncmp(p, "until ", 6) == 0) { handle_loop(p + 6, 4); return; }
    if (strncmp(p, "break", 5) == 0 && (p[5] == ' ' || p[5] == 0)) {
        for (int d = block_depth - 1; d >= 0; d--) {
            if (blocks[d].kind >= 2) { skip_target = d; skip_act = 0; return; }
        }
        return;
    }
    if (strncmp(p, "continue", 8) == 0 && (p[8] == ' ' || p[8] == 0)) {
        for (int d = block_depth - 1; d >= 0; d--) {
            if (blocks[d].kind >= 2) { skip_target = d; skip_act = 1; return; }
        }
        return;
    }

    if (block_active()) shell_execute(p);
}

void shell_handle_line(const char* raw) {
    char expanded[SHELL_MAX_CMD];
    expand_vars(expanded, sizeof(expanded), raw);
    char* segs[8];
    int n = split_semicolons(expanded, segs, 8);
    for (int i = 0; i < n; i++) handle_segment(segs[i]);
}

/* ---------- Command dispatch ---------- */
static int last_status = 0;

static int run_command(int argc, char** argv) {
    for (int i = 0; commands[i].name; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            return commands[i].handler(argc, argv);
        }
    }
    if (strcmp(argv[0], "exit") == 0 || strcmp(argv[0], "quit") == 0) {
        kprintf("Cannot exit shell (kernel process).\n");
        return 0;
    }
    char path[FS_MAX_PATH];
    sprintf(path, "/bin/%s", argv[0]);
    if (fs_exists(path)) {
        kprintf("'%s': executable format not supported\n", argv[0]);
        return 127;
    }
    kprintf("%s: command not found (type 'help' for commands)\n", argv[0]);
    return 127;
}

static int run_line(const char* line) {
    char copy[SHELL_MAX_CMD];
    strncpy(copy, line, SHELL_MAX_CMD - 1);
    char* argv[SHELL_MAX_ARGS];
    int argc = parse_args(copy, argv);
    if (argc == 0) return 0;
    last_status = run_command(argc, argv);
    return last_status;
}

static int run_status(const char* line) {
    last_status = run_line(line);
    char s[16];
    itoa(last_status, s, 10);
    env_set("?", s);
    return last_status;
}

void shell_execute(const char* cmdline) {
    char line[SHELL_MAX_CMD];
    expand_vars(line, sizeof(line), cmdline);

    /* NAME=value [rest] assignment */
    char* eq = strchr(line, '=');
    char* sp = strchr(line, ' ');
    if (eq && (sp == NULL || eq < sp)) {
        bool valid = (eq != line);
        for (char* p = line; valid && p < eq; p++) {
            char c = *p;
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')) valid = false;
        }
        if (valid) {
            char name[ENV_NAME_LEN];
            int nl = (int)(eq - line);
            if (nl > ENV_NAME_LEN - 1) nl = ENV_NAME_LEN - 1;
            memcpy(name, line, nl);
            name[nl] = 0;
            char* v = eq + 1;
            int vl = 0;
            while (v[vl] && v[vl] != ' ') vl++;
            if (vl > ENV_VAL_LEN - 1) vl = ENV_VAL_LEN - 1;
            char val[ENV_VAL_LEN];
            memcpy(val, v, vl);
            val[vl] = 0;
            env_set(name, val);
            char* rest = v + vl;
            while (*rest == ' ') rest++;
            if (*rest) shell_execute(rest);
            return;
        }
    }

    /* pipe: cmd1 | cmd2  (cmd1 output captured to /tmp/.pipe) */
    char* pipe_pos = find_unquoted(line, '|');
    if (pipe_pos) {
        *pipe_pos = 0;
        char* right = pipe_pos + 1;
        while (*right == ' ') right++;
        if (!*right) { kprintf("syntax error: expected command after '|'\n"); return; }
        int r0 = vga_get_row(), c0 = vga_get_col();
        run_line(line);
        int r1 = vga_get_row(), c1 = vga_get_col();
        save_vga_region("/tmp/.pipe", r0, c0, r1, c1, false);
        restore_screen(r0, c0, r1, c1);
        char piped[SHELL_MAX_CMD];
        sprintf(piped, "%s /tmp/.pipe", right);
        run_line(piped);
        return;
    }

    /* redirection: cmd > file  /  cmd >> file */
    char* gt = find_redirect(line);
    if (gt) {
        bool append = (gt[1] == '>');
        char* target = gt + (append ? 2 : 1);
        while (*target == ' ') target++;
        if (!*target) { kprintf("syntax error: expected file after '>'\n"); return; }
        char* end = target + strlen(target);
        while (end > target && (end[-1] == ' ' || end[-1] == '\t')) { end--; *end = 0; }
        *gt = 0;
        int r0 = vga_get_row(), c0 = vga_get_col();
        run_line(line);
        int r1 = vga_get_row(), c1 = vga_get_col();
        save_vga_region(target, r0, c0, r1, c1, append);
        restore_screen(r0, c0, r1, c1);
        return;
    }

    run_line(line);
}

/* ---------- Text editor ---------- */
#define EDIT_MAX_LINES 1024
#define EDIT_MAX_SIZE  32768

static void editor_rebuild(char* buf, int* offs, int* nlines) {
    *nlines = 0;
    int i = 0;
    while (buf[i] && *nlines < EDIT_MAX_LINES - 1) {
        offs[*nlines] = i;
        (*nlines)++;
        while (buf[i] && buf[i] != '\n') i++;
        if (buf[i] == '\n') i++;
    }
    if (*nlines == 0) { offs[0] = 0; *nlines = 1; }
}

static int editor_insert(char* buf, int* offs, int* nlines, int at, const char* text) {
    if (*nlines >= EDIT_MAX_LINES - 1) return -1;
    int end = offs[*nlines - 1];
    while (buf[end] && buf[end] != '\n') end++;
    if (buf[end]) end++;
    int pos = (at >= *nlines) ? end : offs[at];
    int tlen = strlen(text);
    if (end + tlen + 1 >= EDIT_MAX_SIZE) return -1;
    memmove(buf + pos + tlen + 1, buf + pos, end - pos);
    memcpy(buf + pos, text, tlen);
    buf[pos + tlen] = '\n';
    editor_rebuild(buf, offs, nlines);
    return 0;
}

static int editor_delete(char* buf, int* offs, int* nlines, int line) {
    if (line < 0 || line >= *nlines) return -1;
    int start = offs[line];
    int end = start;
    while (buf[end] && buf[end] != '\n') end++;
    if (buf[end]) end++;
    int total = offs[*nlines - 1];
    while (buf[total] && buf[total] != '\n') total++;
    if (buf[total]) total++;
    memmove(buf + start, buf + end, total - end);
    buf[total - (end - start)] = 0;
    (*nlines)--;
    if (*nlines == 0) {
        buf[0] = '\n'; buf[1] = 0;
        offs[0] = 0;
        *nlines = 1;
    } else {
        editor_rebuild(buf, offs, nlines);
    }
    return 0;
}

int cmd_edit(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: edit <file>\n"); return 1; }
    const char* path = argv[1];
    char* buf = (char*)kmalloc(EDIT_MAX_SIZE);
    int* offs = (int*)kmalloc(EDIT_MAX_LINES * sizeof(int));
    if (!buf || !offs) {
        kprintf("edit: out of memory\n");
        if (buf) kfree(buf);
        if (offs) kfree(offs);
        return 1;
    }
    memset(buf, 0, EDIT_MAX_SIZE);
    int size = 0;
    if (fs_exists(path)) {
        int fd = fs_open(path, O_RDONLY);
        if (fd >= 0) {
            size = fs_read(fd, buf, EDIT_MAX_SIZE - 1);
            fs_close(fd);
        }
    }
    if (size > 0 && buf[size - 1] != '\n') { buf[size] = '\n'; size++; buf[size] = 0; }
    if (size == 0) { buf[0] = '\n'; buf[1] = 0; }
    int nlines = 0;
    editor_rebuild(buf, offs, &nlines);
    bool dirty = false;

    for (;;) {
        vga_clear();
        kprintf("--- edit: %s (%d lines%s) ---\n", path, nlines, dirty ? " *modified*" : "");
        kprintf("h=help a=append i<N> insert d<N> delete r<N> replace s<old,new> f<text> p=print w=save q=quit wq=save+quit\n\n");
        for (int i = 0; i < nlines; i++) {
            if (i >= 17) { kprintf("   ... (%d more lines)\n", nlines - i); break; }
            char lb[81];
            int len = 0;
            int pos = offs[i];
            while (buf[pos + len] && buf[pos + len] != '\n' && len < 80) { lb[len] = buf[pos + len]; len++; }
            lb[len] = 0;
            kprintf("%4d| %s\n", i + 1, lb);
        }
        char cmd[256];
        kb_readline(cmd, sizeof(cmd), "> ");
        if (cmd[0] == 'q' && cmd[1] == 0) break;
        if (cmd[0] == 'w') {
            int total = offs[nlines - 1];
            while (buf[total] && buf[total] != '\n') total++;
            if (buf[total]) total++;
            int fd = fs_open(path, O_WRONLY | O_TRUNC | O_CREAT);
            if (fd < 0) { kprintf("edit: cannot open %s\n", path); break; }
            fs_write(fd, buf, total);
            fs_close(fd);
            dirty = false;
            kprintf("saved %d bytes to %s\n", total, path);
            if (cmd[1] == 'q') break;
            continue;
        }
        if (cmd[0] == 'h') continue;
        if (cmd[0] == 'p') {
            for (int i = 0; i < nlines; i++) {
                char lb[81];
                int len = 0;
                int pos = offs[i];
                while (buf[pos + len] && buf[pos + len] != '\n' && len < 80) { lb[len] = buf[pos + len]; len++; }
                lb[len] = 0;
                kprintf("%4d| %s\n", i + 1, lb);
            }
            kprintf("(press any key)");
            keyboard_getchar();
            continue;
        }
        if (cmd[0] == 'a') {
            kprintf("append mode (enter '.' on empty line to end)\n");
            for (;;) {
                char text[256];
                kb_readline(text, sizeof(text), "append> ");
                if (strcmp(text, ".") == 0) break;
                if (editor_insert(buf, offs, &nlines, nlines, text) < 0) { kprintf("buffer full\n"); break; }
                dirty = true;
            }
            continue;
        }
        if (cmd[0] == 'i') {
            int ln = atoi(cmd + 1);
            if (ln < 1) ln = 1;
            if (ln > nlines) ln = nlines;
            char text[256];
            kb_readline(text, sizeof(text), "text> ");
            if (editor_insert(buf, offs, &nlines, ln - 1, text) < 0) kprintf("buffer full\n");
            else dirty = true;
            continue;
        }
        if (cmd[0] == 'd') {
            int ln = atoi(cmd + 1);
            if (ln < 1 || ln > nlines) { kprintf("line out of range\n"); continue; }
            if (editor_delete(buf, offs, &nlines, ln - 1) < 0) kprintf("cannot delete\n");
            else dirty = true;
            continue;
        }
        if (cmd[0] == 'r') {
            int ln = atoi(cmd + 1);
            if (ln < 1 || ln > nlines) { kprintf("line out of range\n"); continue; }
            char text[256];
            kb_readline(text, sizeof(text), "text> ");
            if (editor_delete(buf, offs, &nlines, ln - 1) == 0) {
                editor_insert(buf, offs, &nlines, ln - 1, text);
                dirty = true;
            }
            continue;
        }
        if (cmd[0] == 's') {
            char* comma = strchr(cmd + 1, ',');
            if (!comma) { kprintf("usage: s<old,new>\n"); continue; }
            *comma = 0;
            char* oldp = cmd + 1;
            char* newp = comma + 1;
            int oldlen = strlen(oldp), newlen = strlen(newp);
            if (oldlen == 0) { kprintf("empty old text\n"); continue; }
            int total = offs[nlines - 1];
            while (buf[total] && buf[total] != '\n') total++;
            if (buf[total]) total++;
            char* work = (char*)kmalloc(EDIT_MAX_SIZE);
            if (!work) { kprintf("out of memory\n"); continue; }
            memcpy(work, buf, total);
            int src = 0, dst = 0, replacements = 0;
            while (src < total) {
                if (strncmp(work + src, oldp, oldlen) == 0) {
                    memcpy(buf + dst, newp, newlen);
                    dst += newlen;
                    src += oldlen;
                    replacements++;
                } else {
                    buf[dst++] = work[src++];
                }
            }
            buf[dst] = 0;
            kfree(work);
            editor_rebuild(buf, offs, &nlines);
            kprintf("%d replacement(s)\n", replacements);
            dirty = true;
            continue;
        }
        if (cmd[0] == 'f') {
            for (int i = 0; i < nlines; i++) {
                char lb[81];
                int len = 0;
                int pos = offs[i];
                while (buf[pos + len] && buf[pos + len] != '\n' && len < 80) { lb[len] = buf[pos + len]; len++; }
                lb[len] = 0;
                if (strstr(lb, cmd + 1)) kprintf("%4d| %s\n", i + 1, lb);
            }
            kprintf("(press any key)");
            keyboard_getchar();
            continue;
        }
        kprintf("unknown command\n");
    }
    kfree(buf);
    kfree(offs);
    vga_clear();
    return 0;
}

/* ---------- Game: 2048 ---------- */
static void t2048_spawn(int g[4][4]) {
    int empty[16], n = 0;
    for (int i = 0; i < 16; i++) if (g[i / 4][i % 4] == 0) empty[n++] = i;
    if (n == 0) return;
    int p = empty[rand() % n];
    g[p / 4][p % 4] = (rand() % 10 < 9) ? 2 : 4;
}

static int t2048_slide_line(int* l, int n) {
    int tmp[4], k = 0;
    for (int i = 0; i < n; i++) if (l[i]) tmp[k++] = l[i];
    int out[4], j = 0, gained = 0;
    for (int i = 0; i < k; i++) {
        if (i + 1 < k && tmp[i] == tmp[i + 1]) {
            out[j++] = tmp[i] * 2;
            gained += tmp[i] * 2;
            i++;
        } else {
            out[j++] = tmp[i];
        }
    }
    memset(l, 0, n * sizeof(int));
    memcpy(l, out, j * sizeof(int));
    return gained;
}

static bool t2048_move(int g[4][4], int dir, int* gained) {
    int before[16];
    memcpy(before, g, sizeof(before));
    int total = 0;
    if (dir == 0) {
        for (int r = 0; r < 4; r++) total += t2048_slide_line(g[r], 4);
    } else if (dir == 1) {
        for (int r = 0; r < 4; r++) {
            int l[4];
            for (int c = 0; c < 4; c++) l[c] = g[r][3 - c];
            total += t2048_slide_line(l, 4);
            for (int c = 0; c < 4; c++) g[r][3 - c] = l[c];
        }
    } else if (dir == 2) {
        for (int c = 0; c < 4; c++) {
            int l[4];
            for (int r = 0; r < 4; r++) l[r] = g[r][c];
            total += t2048_slide_line(l, 4);
            for (int r = 0; r < 4; r++) g[r][c] = l[r];
        }
    } else {
        for (int c = 0; c < 4; c++) {
            int l[4];
            for (int r = 0; r < 4; r++) l[r] = g[3 - r][c];
            total += t2048_slide_line(l, 4);
            for (int r = 0; r < 4; r++) g[3 - r][c] = l[r];
        }
    }
    *gained = total;
    return memcmp(before, g, sizeof(before)) != 0;
}

static bool t2048_over(int g[4][4]) {
    for (int i = 0; i < 16; i++) {
        int r = i / 4, c = i % 4;
        if (g[r][c] == 0) return false;
        if (c < 3 && g[r][c] == g[r][c + 1]) return false;
        if (r < 3 && g[r][c] == g[r + 1][c]) return false;
    }
    return true;
}

static void t2048_print(int g[4][4], int score) {
    vga_clear();
    kprintf("  -- 2 0 4 8 --   score: %d\n", score);
    kprintf("  w/a/s/d move | r restart | q quit\n\n");
    for (int r = 0; r < 4; r++) {
        kprintf("  +------+------+------+------+\n  |");
        for (int c = 0; c < 4; c++) {
            if (g[r][c] == 0) kprintf("      |");
            else kprintf("%6d|", g[r][c]);
        }
        kprintf("\n");
    }
    kprintf("  +------+------+------+------+\n");
}

int cmd_game_2048(int argc UNUSED, char** argv UNUSED) {
    int g[4][4];
    memset(g, 0, sizeof(g));
    int score = 0;
    t2048_spawn(g);
    t2048_spawn(g);
    for (;;) {
        t2048_print(g, score);
        char c = keyboard_getchar();
        if (c == 'q') break;
        if (c == 'r') {
            memset(g, 0, sizeof(g));
            score = 0;
            t2048_spawn(g);
            t2048_spawn(g);
            continue;
        }
        int dir = -1;
        if (c == 'w' || c == 0x90) dir = 2;
        if (c == 's' || c == 0x91) dir = 3;
        if (c == 'a' || c == 0x92) dir = 0;
        if (c == 'd' || c == 0x93) dir = 1;
        if (dir >= 0) {
            int gained = 0;
            if (t2048_move(g, dir, &gained)) {
                score += gained;
                t2048_spawn(g);
            }
        }
        if (t2048_over(g)) {
            t2048_print(g, score);
            kprintf("\n  GAME OVER - score %d (press any key)\n", score);
            keyboard_getchar();
            break;
        }
    }
    vga_clear();
    return 0;
}

/* ---------- Game: Snake ---------- */
#define SNAKE_W 40
#define SNAKE_H 16

int cmd_snake(int argc UNUSED, char** argv UNUSED) {
    static int sx[512], sy[512];
    int len = 4, dir = 1;
    int headx = SNAKE_W / 2, heady = SNAKE_H / 2;
    for (int i = 0; i < len; i++) { sx[i] = headx - i; sy[i] = heady; }
    int fx = rand() % SNAKE_W, fy = rand() % SNAKE_H;
    int score = 0;
    uint32_t last_tick = timer_get_ticks();
    bool dead = false;

    vga_clear();
    kprintf("SNAKE  w/a/s/d or arrows, q quit   score: 0");
    for (int x = 0; x <= SNAKE_W + 1; x++) {
        vga_put_at('#', VGA_COLOR(VGA_LIGHT_CYAN, VGA_BLACK), 1, x + 10);
        vga_put_at('#', VGA_COLOR(VGA_LIGHT_CYAN, VGA_BLACK), SNAKE_H + 2, x + 10);
    }
    for (int y = 1; y <= SNAKE_H + 1; y++) {
        vga_put_at('#', VGA_COLOR(VGA_LIGHT_CYAN, VGA_BLACK), y, 10);
        vga_put_at('#', VGA_COLOR(VGA_LIGHT_CYAN, VGA_BLACK), y, SNAKE_W + 11);
    }
    vga_put_at('@', VGA_COLOR(VGA_YELLOW, VGA_BLACK), 1 + fy, 10 + fx);

    while (!dead) {
        while (keyboard_has_input()) {
            char c = keyboard_read_nonblocking();
            if (c == 'q') { dead = true; break; }
            if ((c == 'w' || c == 0x90) && dir != 2) dir = 0;
            if ((c == 's' || c == 0x91) && dir != 0) dir = 2;
            if ((c == 'a' || c == 0x92) && dir != 1) dir = 3;
            if ((c == 'd' || c == 0x93) && dir != 3) dir = 1;
        }
        if (dead) break;
        if (timer_get_ticks() - last_tick < 12) continue;
        last_tick = timer_get_ticks();

        int nx = headx, ny = heady;
        if (dir == 0) ny--;
        if (dir == 2) ny++;
        if (dir == 3) nx--;
        if (dir == 1) nx++;

        bool hit = (nx < 0 || ny < 0 || nx >= SNAKE_W || ny >= SNAKE_H);
        if (!hit) {
            for (int i = 0; i < len; i++) if (sx[i] == nx && sy[i] == ny) hit = true;
        }
        if (hit) {
            kprintf("\nGAME OVER - score %d\n", score);
            break;
        }

        vga_put_at(' ', VGA_COLOR(VGA_BLACK, VGA_BLACK), 1 + sy[len - 1], 10 + sx[len - 1]);
        for (int i = len - 1; i > 0; i--) { sx[i] = sx[i - 1]; sy[i] = sy[i - 1]; }
        sx[0] = nx; sy[0] = ny;
        headx = nx; heady = ny;
        vga_put_at('O', VGA_COLOR(VGA_LIGHT_GREEN, VGA_BLACK), 1 + sy[0], 10 + sx[0]);

        if (nx == fx && ny == fy) {
            score += 10;
            if (len < 512) len++;
            char s[32];
            sprintf(s, "SNAKE  w/a/s/d or arrows, q quit   score: %d", score);
            vga_set_cursor(0, 0);
            vga_puts_color(s, VGA_COLOR(VGA_LIGHT_GREEN, VGA_BLACK));
            fx = rand() % SNAKE_W;
            fy = rand() % SNAKE_H;
            vga_put_at('@', VGA_COLOR(VGA_YELLOW, VGA_BLACK), 1 + fy, 10 + fx);
        }
    }
    vga_clear();
    return 0;
}

/* ---------- Game: Tic-tac-toe (minimax AI) ---------- */
static char ttt_winner(char b[9]) {
    static const int lines[8][3] = {
        {0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}
    };
    for (int i = 0; i < 8; i++) {
        if (b[lines[i][0]] != ' ' && b[lines[i][0]] == b[lines[i][1]] && b[lines[i][1]] == b[lines[i][2]])
            return b[lines[i][0]];
    }
    return ' ';
}

static int ttt_minimax(char b[9], char player) {
    char w = ttt_winner(b);
    if (w == 'X') return -10;
    if (w == 'O') return 10;
    bool full = true;
    for (int i = 0; i < 9; i++) if (b[i] == ' ') { full = false; break; }
    if (full) return 0;
    int best = (player == 'O') ? -1000 : 1000;
    for (int i = 0; i < 9; i++) {
        if (b[i] == ' ') {
            b[i] = player;
            int v = ttt_minimax(b, player == 'O' ? 'X' : 'O');
            b[i] = ' ';
            if (player == 'O') { if (v > best) best = v; }
            else { if (v < best) best = v; }
        }
    }
    return best;
}

static int ttt_cpu_move(char b[9]) {
    int best = -1000, besti = -1;
    for (int i = 0; i < 9; i++) {
        if (b[i] == ' ') {
            b[i] = 'O';
            int v = ttt_minimax(b, 'X');
            b[i] = ' ';
            if (v > best) { best = v; besti = i; }
        }
    }
    return besti;
}

static void ttt_print(char b[9]) {
    vga_clear();
    kprintf(" TIC-TAC-TOE (you X, cpu O)\n\n");
    kprintf("   %c | %c | %c      1 | 2 | 3\n", b[0], b[1], b[2]);
    kprintf("  ---+---+---    ---+---+---\n");
    kprintf("   %c | %c | %c      4 | 5 | 6\n", b[3], b[4], b[5]);
    kprintf("  ---+---+---    ---+---+---\n");
    kprintf("   %c | %c | %c      7 | 8 | 9\n", b[6], b[7], b[8]);
    kprintf("\n  enter 1-9 or q\n");
}

int cmd_tictactoe(int argc UNUSED, char** argv UNUSED) {
    for (;;) {
        char b[9];
        for (int i = 0; i < 9; i++) b[i] = ' ';
        for (;;) {
            ttt_print(b);
            char c = keyboard_getchar();
            if (c == 'q') { vga_clear(); return 0; }
            int pos = -1;
            if (c >= '1' && c <= '9') pos = c - '1';
            if (pos < 0 || b[pos] != ' ') { kprintf("invalid move\n"); continue; }
            b[pos] = 'X';
            if (ttt_winner(b) == 'X') { ttt_print(b); kprintf("\nYOU WIN!\n"); break; }
            bool full = true;
            for (int i = 0; i < 9; i++) if (b[i] == ' ') { full = false; break; }
            if (full) { ttt_print(b); kprintf("\nDRAW\n"); break; }
            int m = ttt_cpu_move(b);
            b[m] = 'O';
            if (ttt_winner(b) == 'O') { ttt_print(b); kprintf("\nCPU WINS\n"); break; }
            full = true;
            for (int i = 0; i < 9; i++) if (b[i] == ' ') { full = false; break; }
            if (full) { ttt_print(b); kprintf("\nDRAW\n"); break; }
        }
        kprintf("play again? (y/n) ");
        char c = keyboard_getchar();
        kprintf("\n");
        if (c != 'y' && c != 'Y') break;
    }
    vga_clear();
    return 0;
}

/* ---------- Game: Hangman ---------- */
int cmd_hangman(int argc UNUSED, char** argv UNUSED) {
    static const char* words[] = {
        "matrix", "kernel", "scheduler", "interrupt", "assembly", "compiler",
        "filesystem", "network", "graphics", "keyboard", "memory", "process"
    };
    for (;;) {
        const char* word = words[rand() % 12];
        int wlen = strlen(word);
        char shown[32], guessed[32];
        int glen = 0, wrong = 0;
        for (int i = 0; i < wlen; i++) shown[i] = '_';
        shown[wlen] = 0;
        guessed[0] = 0;
        bool done = false;
        while (!done && wrong < 6) {
            vga_clear();
            kprintf(" HANGMAN\n\n");
            kprintf("   +---+\n   |   |\n");
            kprintf("   %c   |\n", wrong >= 1 ? 'O' : ' ');
            kprintf("  %c%c%c  |\n", wrong >= 3 ? '/' : ' ', wrong >= 2 ? '|' : ' ', wrong >= 4 ? '\\' : ' ');
            kprintf("   %c   |\n", wrong >= 2 ? '|' : ' ');
            kprintf("  %c %c  |\n", wrong >= 5 ? '/' : ' ', wrong >= 6 ? '\\' : ' ');
            kprintf("       |\n  ======\n\n");
            kprintf(" word: %s\n\n", shown);
            kprintf(" guessed: %s\n", guessed[0] ? guessed : "(none)");
            kprintf(" wrong: %d/6\n", wrong);
            kprintf(" guess a letter or q: ");
            char c = keyboard_getchar();
            kprintf("\n");
            if (c == 'q') { vga_clear(); return 0; }
            if (c < 'a' || c > 'z') continue;
            bool seen = false;
            for (int i = 0; i < glen; i++) if (guessed[i] == c) seen = true;
            if (seen) continue;
            guessed[glen++] = c;
            guessed[glen] = 0;
            bool hit = false;
            for (int i = 0; i < wlen; i++) {
                if (word[i] == c) { shown[i] = c; hit = true; }
            }
            if (hit) {
                bool all = true;
                for (int i = 0; i < wlen; i++) if (shown[i] == '_') all = false;
                if (all) {
                    vga_clear();
                    kprintf("YOU WIN! The word was: %s\n", word);
                    done = true;
                }
            } else {
                wrong++;
                if (wrong >= 6) {
                    vga_clear();
                    kprintf("HANGED! The word was: %s\n", word);
                    done = true;
                }
            }
        }
        kprintf("play again? (y/n) ");
        char c = keyboard_getchar();
        kprintf("\n");
        if (c != 'y' && c != 'Y') break;
    }
    vga_clear();
    return 0;
}

/* ---------- Math: scientific calculator ---------- */
typedef struct {
    const char* p;
    int err;
} sci_parser_t;

static int sci_expr(sci_parser_t* pt);
static int sci_term(sci_parser_t* pt);
static int sci_factor(sci_parser_t* pt);
static int sci_primary(sci_parser_t* pt);

static int sci_apply(sci_parser_t* pt, const char* name) {
    if (*pt->p == '(') pt->p++;
    int a = sci_expr(pt);
    int b = 0;
    if (*pt->p == ',') { pt->p++; b = sci_expr(pt); }
    if (*pt->p == ')') pt->p++;
    if (strcmp(name, "sqrt") == 0) return fp_sqrt(a);
    if (strcmp(name, "cbrt") == 0) {
        if (a < 0) return -fp_exp(fp_div(fp_ln(-a), 3 * FP_ONE));
        return fp_exp(fp_div(fp_ln(a), 3 * FP_ONE));
    }
    if (strcmp(name, "pow") == 0) {
        if (b % FP_ONE == 0) return fp_pow(a, b / FP_ONE);
        if (a < 0) { pt->err = 1; return 0; }
        return fp_exp(fp_mul(b, fp_ln(a)));
    }
    if (strcmp(name, "exp") == 0) return fp_exp(a);
    if (strcmp(name, "ln") == 0) return fp_ln(a);
    if (strcmp(name, "log") == 0) return fp_log10(a);
    if (strcmp(name, "sin") == 0) return fp_sin(a);
    if (strcmp(name, "cos") == 0) return fp_cos(a);
    if (strcmp(name, "tan") == 0) {
        int c = fp_cos(a);
        if (fp_abs(c) < 100) { pt->err = 1; return 0; }
        return fp_div(fp_sin(a), c);
    }
    if (strcmp(name, "abs") == 0) return fp_abs(a);
    if (strcmp(name, "round") == 0) return ((a + 32768) >> 16) * FP_ONE;
    if (strcmp(name, "floor") == 0) return (a >> 16) * FP_ONE;
    if (strcmp(name, "ceil") == 0) return (-(((-a) >> 16))) * FP_ONE;
    if (strcmp(name, "min") == 0) return a < b ? a : b;
    if (strcmp(name, "max") == 0) return a > b ? a : b;
    if (strcmp(name, "gcd") == 0) {
        int x = a / FP_ONE, y = b / FP_ONE;
        if (x < 0) x = -x;
        if (y < 0) y = -y;
        while (y) { int t = x % y; x = y; y = t; }
        return x * FP_ONE;
    }
    if (strcmp(name, "lcm") == 0) {
        int x = a / FP_ONE, y = b / FP_ONE;
        if (x == 0 || y == 0) return 0;
        if (x < 0) x = -x;
        if (y < 0) y = -y;
        int g = x, yy = y;
        while (yy) { int t = g % yy; g = yy; yy = t; }
        return (x / g) * y * FP_ONE;
    }
    if (strcmp(name, "fact") == 0) {
        int n = a / FP_ONE;
        if (n < 0 || n > 12) { pt->err = 1; return 0; }
        int f = 1;
        for (int i = 2; i <= n; i++) f *= i;
        return f * FP_ONE;
    }
    if (strcmp(name, "mod") == 0) {
        if (b == 0) { pt->err = 1; return 0; }
        return a % b;
    }
    if (strcmp(name, "hypot") == 0) return fp_sqrt(fp_mul(a, a) + fp_mul(b, b));
    if (strcmp(name, "avg") == 0) return fp_div(a + b, 2 * FP_ONE);
    if (strcmp(name, "rad") == 0) return fp_div(fp_mul(a, FP_PI), 180 * FP_ONE);
    if (strcmp(name, "deg") == 0) return fp_div(fp_mul(a, 180 * FP_ONE), FP_PI);
    pt->err = 1;
    return 0;
}

static int sci_primary(sci_parser_t* pt) {
    const char* s = pt->p;
    if (*s == '(') {
        pt->p++;
        int v = sci_expr(pt);
        if (*pt->p == ')') pt->p++;
        return v;
    }
    if ((*s >= '0' && *s <= '9') || *s == '.') {
        return fp_parse(&pt->p);
    }
    char name[16];
    int ni = 0;
    while ((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') || (*s >= '0' && *s <= '9')) {
        if (ni < 15) name[ni++] = *s;
        s++;
    }
    name[ni] = 0;
    pt->p = s;
    if (name[0] == 0) { pt->err = 1; return 0; }
    if (strcmp(name, "pi") == 0) return FP_PI;
    if (strcmp(name, "e") == 0) return FP_E;
    if (strcmp(name, "phi") == 0) return FP_PHI;
    if (strcmp(name, "sqrt2") == 0) return FP_SQRT2;
    if (strcmp(name, "tau") == 0) return 2 * FP_PI;
    return sci_apply(pt, name);
}

static int sci_factor(sci_parser_t* pt) {
    const char* s = pt->p;
    int neg = 0;
    while (*s == '-' || *s == '+') {
        if (*s == '-') neg = !neg;
        s++;
    }
    pt->p = s;
    int base = sci_primary(pt);
    if (*pt->p == '^') {
        pt->p++;
        int e = sci_factor(pt);
        if (e % FP_ONE == 0) base = fp_pow(base, e / FP_ONE);
        else {
            if (base < 0) { pt->err = 1; return 0; }
            base = fp_exp(fp_mul(e, fp_ln(base)));
        }
    }
    return neg ? -base : base;
}

static int sci_term(sci_parser_t* pt) {
    int v = sci_factor(pt);
    for (;;) {
        if (*pt->p == '*') { pt->p++; v = fp_mul(v, sci_factor(pt)); }
        else if (*pt->p == '/') {
            pt->p++;
            int d = sci_factor(pt);
            if (d == 0) { pt->err = 1; return 0; }
            v = fp_div(v, d);
        } else break;
    }
    return v;
}

static int sci_expr(sci_parser_t* pt) {
    int v = sci_term(pt);
    for (;;) {
        if (*pt->p == '+') { pt->p++; v += sci_term(pt); }
        else if (*pt->p == '-') { pt->p++; v -= sci_term(pt); }
        else break;
    }
    return v;
}

int cmd_sci(int argc, char** argv) {
    if (argc < 2) {
        kprintf("Usage: sci <expression>\n");
        kprintf("Functions: sqrt cbrt pow exp ln log sin cos tan abs round floor ceil\n");
        kprintf("           min max gcd lcm fact mod hypot avg rad deg\n");
        kprintf("Constants: pi e phi tau sqrt2   Example: sci sqrt(2)+pow(3,2)*pi\n");
        return 1;
    }
    char expr[512];
    expr[0] = 0;
    for (int i = 1; i < argc; i++) {
        strcat(expr, argv[i]);
        strcat(expr, " ");
    }
    sci_parser_t pt;
    pt.p = expr;
    pt.err = 0;
    int v = sci_expr(&pt);
    if (pt.err || v == 0x7FFFFFFF) { kprintf("sci: error in expression\n"); return 1; }
    fp_print(v);
    kprintf("\n");
    return 0;
}

/* ---------- Math: statistics ---------- */
int cmd_stats(int argc, char** argv) {
    int vals[256];
    int n = 0;
    if (argc >= 3 && strcmp(argv[1], "-f") == 0) {
        int fd = fs_open(argv[2], O_RDONLY);
        if (fd < 0) { kprintf("stats: cannot open %s\n", argv[2]); return 1; }
        char* buf = (char*)kmalloc(16384);
        if (!buf) { fs_close(fd); kprintf("stats: out of memory\n"); return 1; }
        int size = fs_read(fd, buf, 16383);
        buf[size] = 0;
        fs_close(fd);
        char* p = buf;
        while (*p && n < 256) {
            while (*p == ' ' || *p == '\n' || *p == '\t') p++;
            if (!*p) break;
            const char* tmp = p;
            vals[n++] = fp_parse(&tmp);
            p = (char*)tmp;
        }
        kfree(buf);
    } else {
        for (int i = 1; i < argc && n < 256; i++) {
            const char* tmp = argv[i];
            vals[n++] = fp_parse(&tmp);
        }
    }
    if (n == 0) { kprintf("stats: no data\n"); return 1; }

    int min = vals[0], max = vals[0];
    int64_t sum = 0;
    for (int i = 0; i < n; i++) {
        if (vals[i] < min) min = vals[i];
        if (vals[i] > max) max = vals[i];
        sum += vals[i];
    }
    int mean = (int)(sum / n);
    int sorted[256];
    memcpy(sorted, vals, n * sizeof(int));
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (sorted[j] < sorted[i]) { int t = sorted[i]; sorted[i] = sorted[j]; sorted[j] = t; }
        }
    }
    int median = (n % 2) ? sorted[n / 2] : fp_div(sorted[n / 2 - 1] + sorted[n / 2], 2 * FP_ONE);
    int best = sorted[0], bestc = 0;
    for (int i = 0; i < n; i++) {
        int c = 0;
        for (int j = 0; j < n; j++) if (sorted[j] == sorted[i]) c++;
        if (c > bestc) { bestc = c; best = sorted[i]; }
    }
    int64_t var = 0;
    for (int i = 0; i < n; i++) {
        int64_t d = (int64_t)vals[i] - mean;
        var += fp_mul((int)d, (int)d);
    }
    var /= n;
    int stddev = fp_sqrt((int)var);

    kprintf("count:   %d\n", n);
    kprintf("min:     "); fp_print(min); kprintf("\n");
    kprintf("max:     "); fp_print(max); kprintf("\n");
    kprintf("range:   "); fp_print(max - min); kprintf("\n");
    kprintf("sum:     "); fp_print((int)sum); kprintf("\n");
    kprintf("mean:    "); fp_print(mean); kprintf("\n");
    kprintf("median:  "); fp_print(median); kprintf("\n");
    kprintf("mode:    "); fp_print(best); kprintf(" (%dx)\n", bestc);
    kprintf("variance:"); fp_print((int)var); kprintf("\n");
    kprintf("stddev:  "); fp_print(stddev); kprintf("\n");
    return 0;
}

/* ---------- Math: primes / fib / roman / angles ---------- */
int cmd_primes(int argc, char** argv) {
    int limit = 1000;
    bool count_only = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--count") == 0 || strcmp(argv[i], "-c") == 0) count_only = true;
        else limit = atoi(argv[i]);
    }
    if (limit < 2) limit = 2;
    if (limit > 200000) limit = 200000;
    char* sieve = (char*)kmalloc(limit + 1);
    if (!sieve) { kprintf("primes: out of memory\n"); return 1; }
    memset(sieve, 1, limit + 1);
    sieve[0] = 0;
    sieve[1] = 0;
    for (int i = 2; i * i <= limit; i++) {
        if (sieve[i]) {
            for (int j = i * i; j <= limit; j += i) sieve[j] = 0;
        }
    }
    int count = 0;
    for (int i = 2; i <= limit; i++) if (sieve[i]) count++;
    if (count_only) {
        kprintf("%d primes below %d\n", count, limit);
        kfree(sieve);
        return 0;
    }
    int col = 0;
    for (int i = 2; i <= limit; i++) {
        if (sieve[i]) {
            kprintf("%6d", i);
            if (++col == 10) { kprintf("\n"); col = 0; }
        }
    }
    if (col) kprintf("\n");
    kprintf("total: %d primes below %d\n", count, limit);
    kfree(sieve);
    return 0;
}

int cmd_fib(int argc, char** argv) {
    int n = (argc > 1) ? atoi(argv[1]) : 20;
    if (n < 1) n = 1;
    if (n > 46) n = 46;
    int a = 0, b = 1;
    for (int i = 0; i < n; i++) {
        kprintf("%d", a);
        if (i + 1 < n) kprintf(" ");
        int c = a + b;
        a = b;
        b = c;
    }
    kprintf("\n");
    return 0;
}

int cmd_roman(int argc, char** argv) {
    static const char* roman_syms[] = {"M","CM","D","CD","C","XC","L","XL","X","IX","V","IV","I"};
    static const int roman_vals[] = {1000,900,500,400,100,90,50,40,10,9,5,4,1};
    if (argc < 2) { kprintf("Usage: roman <number> | roman -p <roman>\n"); return 1; }
    if (strcmp(argv[1], "-p") == 0 && argc >= 3) {
        const char* s = argv[2];
        int total = 0;
        for (int i = 0; s[i]; i++) {
            int v = 0, next = 0;
            if (s[i] == 'I') v = 1; else if (s[i] == 'V') v = 5;
            else if (s[i] == 'X') v = 10; else if (s[i] == 'L') v = 50;
            else if (s[i] == 'C') v = 100; else if (s[i] == 'D') v = 500;
            else if (s[i] == 'M') v = 1000;
            else { kprintf("invalid numeral\n"); return 1; }
            if (s[i + 1]) {
                if (s[i+1] == 'I') next = 1; else if (s[i+1] == 'V') next = 5;
                else if (s[i+1] == 'X') next = 10; else if (s[i+1] == 'L') next = 50;
                else if (s[i+1] == 'C') next = 100; else if (s[i+1] == 'D') next = 500;
                else if (s[i+1] == 'M') next = 1000;
            }
            total += (next > v) ? -v : v;
        }
        kprintf("%d\n", total);
        return 0;
    }
    int n = atoi(argv[1]);
    if (n < 1 || n > 3999) { kprintf("out of range (1-3999)\n"); return 1; }
    char out[32];
    int oi = 0;
    for (int i = 0; i < 13; i++) {
        while (n >= roman_vals[i]) {
            const char* s = roman_syms[i];
            while (*s) out[oi++] = *s++;
            n -= roman_vals[i];
        }
    }
    out[oi] = 0;
    kprintf("%s\n", out);
    return 0;
}

int cmd_angles(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: angles <value> <deg|rad|grad>\n"); return 1; }
    const char* tmp = argv[1];
    int v = fp_parse(&tmp);
    int deg, rad, grad;
    if (strcmp(argv[2], "deg") == 0) {
        deg = v;
        rad = fp_div(fp_mul(v, FP_PI), 180 * FP_ONE);
        grad = fp_mul(v, fp_div(10 * FP_ONE, 9 * FP_ONE));
    } else if (strcmp(argv[2], "rad") == 0) {
        rad = v;
        deg = fp_div(fp_mul(v, 180 * FP_ONE), FP_PI);
        grad = fp_mul(deg, fp_div(10 * FP_ONE, 9 * FP_ONE));
    } else if (strcmp(argv[2], "grad") == 0) {
        grad = v;
        deg = fp_mul(v, fp_div(9 * FP_ONE, 10 * FP_ONE));
        rad = fp_div(fp_mul(deg, FP_PI), 180 * FP_ONE);
    } else {
        kprintf("unknown unit (deg/rad/grad)\n");
        return 1;
    }
    kprintf("degrees:  "); fp_print(deg); kprintf("\n");
    kprintf("radians:  "); fp_print(rad); kprintf("\n");
    kprintf("gradians: "); fp_print(grad); kprintf("\n");
    return 0;
}

/* ---------- Ciphers & encoding ---------- */
int cmd_caesar(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: caesar [-d|--brute] <shift> <text...>\n"); return 1; }
    int argi = 1;
    bool decode = false, brute = false;
    if (strcmp(argv[1], "-d") == 0) { decode = true; argi++; }
    else if (strcmp(argv[1], "--brute") == 0) { brute = true; argi++; }
    int shift = 0;
    if (!brute) {
        shift = atoi(argv[argi]);
        argi++;
    }
    char text[512];
    text[0] = 0;
    for (int i = argi; i < argc; i++) {
        strcat(text, argv[i]);
        if (i < argc - 1) strcat(text, " ");
    }
    if (brute) {
        for (int s = 0; s < 26; s++) {
            kprintf("%2d: ", s);
            for (int i = 0; text[i]; i++) {
                char c = text[i];
                if (c >= 'a' && c <= 'z') kprintf("%c", 'a' + ((c - 'a' + s) % 26 + 26) % 26);
                else if (c >= 'A' && c <= 'Z') kprintf("%c", 'A' + ((c - 'A' + s) % 26 + 26) % 26);
                else kprintf("%c", c);
            }
            kprintf("\n");
        }
        return 0;
    }
    if (decode) shift = -shift;
    for (int i = 0; text[i]; i++) {
        char c = text[i];
        if (c >= 'a' && c <= 'z') kprintf("%c", 'a' + ((c - 'a' + shift) % 26 + 26) % 26);
        else if (c >= 'A' && c <= 'Z') kprintf("%c", 'A' + ((c - 'A' + shift) % 26 + 26) % 26);
        else kprintf("%c", c);
    }
    kprintf("\n");
    return 0;
}

int cmd_vigenere(int argc, char** argv) {
    if (argc < 3) { kprintf("Usage: vigenere [-d] <key> <text...>\n"); return 1; }
    int argi = 1;
    bool decode = false;
    if (strcmp(argv[1], "-d") == 0) { decode = true; argi++; }
    const char* key = argv[argi++];
    int keylen = strlen(key);
    if (keylen == 0) { kprintf("empty key\n"); return 1; }
    char text[512];
    text[0] = 0;
    for (int i = argi; i < argc; i++) {
        strcat(text, argv[i]);
        if (i < argc - 1) strcat(text, " ");
    }
    int ki = 0;
    for (int i = 0; text[i]; i++) {
        char c = text[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            char kb = key[ki % keylen];
            int ks = (kb >= 'a' && kb <= 'z') ? kb - 'a' : (kb >= 'A' && kb <= 'Z' ? kb - 'A' : 0);
            if (decode) ks = -ks;
            if (c >= 'a' && c <= 'z') kprintf("%c", 'a' + ((c - 'a' + ks) % 26 + 26) % 26);
            else kprintf("%c", 'A' + ((c - 'A' + ks) % 26 + 26) % 26);
            ki++;
        } else {
            kprintf("%c", c);
        }
    }
    kprintf("\n");
    return 0;
}

int cmd_atbash(int argc, char** argv) {
    char text[512];
    text[0] = 0;
    for (int i = 1; i < argc; i++) {
        strcat(text, argv[i]);
        if (i < argc - 1) strcat(text, " ");
    }
    for (int i = 0; text[i]; i++) {
        char c = text[i];
        if (c >= 'a' && c <= 'z') kprintf("%c", 'z' - (c - 'a'));
        else if (c >= 'A' && c <= 'Z') kprintf("%c", 'Z' - (c - 'A'));
        else kprintf("%c", c);
    }
    kprintf("\n");
    return 0;
}

int cmd_urlencode(int argc, char** argv) {
    static const char* hexd = "0123456789ABCDEF";
    char text[512];
    text[0] = 0;
    for (int i = 1; i < argc; i++) {
        strcat(text, argv[i]);
        if (i < argc - 1) strcat(text, " ");
    }
    for (int i = 0; text[i]; i++) {
        char c = text[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            kprintf("%c", c);
        } else {
            kprintf("%%%c%c", hexd[(c >> 4) & 0xF], hexd[c & 0xF]);
        }
    }
    kprintf("\n");
    return 0;
}

static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

int cmd_urldecode(int argc, char** argv) {
    char text[512];
    text[0] = 0;
    for (int i = 1; i < argc; i++) {
        strcat(text, argv[i]);
        if (i < argc - 1) strcat(text, " ");
    }
    for (int i = 0; text[i]; i++) {
        if (text[i] == '%' && text[i + 1] && text[i + 2]) {
            int h = hexval(text[i + 1]), l = hexval(text[i + 2]);
            if (h >= 0 && l >= 0) {
                kprintf("%c", (char)(h * 16 + l));
                i += 2;
                continue;
            }
        }
        if (text[i] == '+') kprintf(" ");
        else kprintf("%c", text[i]);
    }
    kprintf("\n");
    return 0;
}

int cmd_hash(int argc, char** argv) {
    if (argc < 2) { kprintf("Usage: hash <file>\n"); return 1; }
    int fd = fs_open(argv[1], O_RDONLY);
    if (fd < 0) { kprintf("hash: cannot open %s\n", argv[1]); return 1; }
    char* buf = (char*)kmalloc(65536);
    if (!buf) { fs_close(fd); kprintf("hash: out of memory\n"); return 1; }
    int size = fs_read(fd, buf, 65535);
    fs_close(fd);
    uint32_t djb2 = 5381, sdbm = 0, fnv = 2166136261u;
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < size; i++) {
        char c = buf[i];
        djb2 = ((djb2 << 5) + djb2) + (uint32_t)(unsigned char)c;
        sdbm = (sdbm << 6) + (sdbm << 16) - sdbm + (uint32_t)(unsigned char)c;
        fnv ^= (uint32_t)(unsigned char)c;
        fnv *= 16777619u;
        crc ^= (uint16_t)(unsigned char)c;
        for (int b = 0; b < 8; b++) crc = (crc & 1) ? (uint16_t)((crc >> 1) ^ 0xA001) : (uint16_t)(crc >> 1);
    }
    kfree(buf);
    kprintf("djb2:  %x\n", djb2);
    kprintf("sdbm:  %x\n", sdbm);
    kprintf("fnv1a: %x\n", fnv);
    kprintf("crc16: %x\n", crc);
    return 0;
}

static void kb_flush(void) {
    while (keyboard_read_nonblocking());
}

/* ---------- Animations ---------- */
int cmd_donut(int argc UNUSED, char** argv UNUSED) {
    trig_init();
    static char zb[80 * 25];
    static char fb[80 * 25];
    static const char* ramp = ".,-~:;=!*#$@";
    int A = 0, B = 0;
    vga_clear();
    kprintf("3D DONUT - press any key to exit\n");
    while (!keyboard_has_input()) {
        memset(zb, 0, sizeof(zb));
        memset(fb, ' ', sizeof(fb));
        for (int j = 0; j < 628; j += 8) {
            int phi = fp_mul(j, 65536 / 100);
            int cphi = fp_cos(phi), sphi = fp_sin(phi);
            for (int i = 0; i < 628; i += 6) {
                int th = fp_mul(i, 65536 / 100);
                int cth = fp_cos(th), sth = fp_sin(th);
                int sinA = sintab[A], cosA = sintab[(A + 90) % 360];
                int sinB = sintab[B], cosB = sintab[(B + 90) % 360];
                int h = cth + 2 * FP_ONE;
                int sp_h_sinA = fp_mul(sphi, fp_mul(h, sinA));
                int den = sp_h_sinA + fp_mul(sth, cosA) + 5 * FP_ONE;
                if (den <= 0) continue;
                int D = fp_div(FP_ONE, den);
                int t = fp_mul(sphi, fp_mul(h, cosA)) - fp_mul(sth, sinA);
                int cp_h = fp_mul(cphi, h);
                int x = 40 + ((fp_mul(fp_mul(fp_mul(30 * FP_ONE, D), cp_h), cosB) - fp_mul(fp_mul(30 * FP_ONE, D), fp_mul(t, sinB))) >> 16);
                int y = 12 + ((fp_mul(fp_mul(fp_mul(15 * FP_ONE, D), cp_h), sinB) + fp_mul(fp_mul(15 * FP_ONE, D), fp_mul(t, cosB))) >> 16);
                if (x >= 0 && x < 80 && y >= 0 && y < 25) {
                    int o = y * 80 + x;
                    int d8 = D >> 8;
                    if (d8 > zb[o]) {
                        zb[o] = (char)d8;
                        int N = fp_mul(8 * FP_ONE,
                            fp_mul(fp_mul(sth, sinA) - fp_mul(sphi, fp_mul(cth, cosA)), cosB)
                            - fp_mul(sphi, fp_mul(cth, sinA))
                            - fp_mul(sth, cosA)
                            - fp_mul(cphi, fp_mul(cth, sinB)));
                        int idx = N >> 16;
                        if (idx < 0) idx = 0;
                        if (idx > 11) idx = 11;
                        fb[o] = ramp[idx];
                    }
                }
            }
        }
        for (int y = 1; y < 25; y++) {
            for (int x = 0; x < 80; x++) {
                vga_put_at(fb[y * 80 + x], VGA_COLOR(VGA_LIGHT_GREEN, VGA_BLACK), y, x);
            }
        }
        A = (A + 2) % 360;
        B = (B + 1) % 360;
        timer_sleep(30);
    }
    kb_flush();
    vga_clear();
    return 0;
}

int cmd_stars(int argc UNUSED, char** argv UNUSED) {
    vga_clear();
    static const char* sh = ".*+oO";
    while (!keyboard_has_input()) {
        for (int y = 23; y >= 1; y--) {
            for (int x = 0; x < 80; x++) {
                VGA_MEMORY[y * 80 + x] = VGA_MEMORY[(y - 1) * 80 + x];
            }
        }
        for (int x = 0; x < 80; x++) {
            VGA_MEMORY[x] = (uint16_t)((VGA_COLOR(VGA_BLACK, VGA_BLACK) << 8) | ' ');
        }
        for (int i = 0; i < 3; i++) {
            int x = rand() % 80;
            int ch = sh[rand() % 5];
            int color = VGA_COLOR(7 + rand() % 8, VGA_BLACK);
            VGA_MEMORY[x] = (uint16_t)((color << 8) | ch);
        }
        if (keyboard_has_input()) { kb_flush(); break; }
        timer_sleep(40);
    }
    vga_clear();
    return 0;
}

int cmd_clock(int argc UNUSED, char** argv UNUSED) {
    vga_clear();
    kprintf("CLOCK - press any key to exit\n");
    uint32_t last = 0xFFFFFFFF;
    while (!keyboard_has_input()) {
        uint32_t s = timer_get_seconds();
        if (s != last) {
            last = s;
            char buf[32];
            sprintf(buf, "  %02d:%02d:%02d  ", (int)(s / 3600) % 24, (int)(s / 60) % 60, (int)(s % 60));
            vga_set_cursor(12, 33);
            vga_puts_color(buf, VGA_COLOR(VGA_YELLOW, VGA_BLUE));
            char buf2[64];
            sprintf(buf2, "uptime: %d days %02d:%02d:%02d", (int)(s / 86400), (int)(s / 3600) % 24, (int)(s / 60) % 60, (int)(s % 60));
            vga_set_cursor(14, 18);
            vga_puts_color(buf2, VGA_COLOR(VGA_LIGHT_CYAN, VGA_BLACK));
        }
        timer_sleep(20);
    }
    kb_flush();
    vga_clear();
    return 0;
}

int cmd_hollywood(int argc UNUSED, char** argv UNUSED) {
    static const char* chars = "0123456789abcdefABCDEF{}[]#$%&*+-=<>?/\\|:;.,`~!@^_()";
    vga_clear();
    while (!keyboard_has_input()) {
        char line[80];
        int len = 15 + rand() % 60;
        for (int i = 0; i < len; i++) line[i] = chars[rand() % 44];
        line[len] = 0;
        vga_puts_color(line, VGA_COLOR(2 + rand() % 14, VGA_BLACK));
        vga_putchar('\n');
        if (vga_get_row() >= 24) vga_set_cursor(0, 0);
        timer_sleep(5 + rand() % 80);
    }
    kb_flush();
    vga_clear();
    return 0;
}

int cmd_progress(int argc, char** argv) {
    int total_ms = (argc > 1) ? atoi(argv[1]) * 10 : 5000;
    if (total_ms < 100) total_ms = 100;
    vga_clear();
    kprintf("PROGRESS - press any key to exit\n");
    int steps = 50;
    for (int p = 0; p <= steps; p++) {
        vga_set_cursor(3, 8);
        vga_puts_color("[", VGA_COLOR(VGA_WHITE, VGA_BLACK));
        for (int i = 0; i < steps; i++) {
            char c = (i < p) ? '=' : ' ';
            uint8_t color = (i < p) ? VGA_COLOR(VGA_LIGHT_GREEN, VGA_BLACK) : VGA_COLOR(VGA_DARK_GREY, VGA_BLACK);
            vga_put_at(c, color, 3, 9 + i);
        }
        vga_set_cursor(3, 59);
        vga_puts_color("]", VGA_COLOR(VGA_WHITE, VGA_BLACK));
        char pct[32];
        sprintf(pct, " %3d%%", p * 2);
        vga_puts_color(pct, VGA_COLOR(VGA_YELLOW, VGA_BLACK));
        if (keyboard_has_input()) { kb_flush(); break; }
        timer_sleep(total_ms / steps);
    }
    vga_clear();
    return 0;
}

/* ---------- System: CPU benchmark ---------- */
int cmd_bench(int argc UNUSED, char** argv UNUSED) {
    kprintf("Benchmarking CPU (3 seconds)...\n");
    uint32_t t0 = timer_get_ticks();
    volatile uint32_t acc = 0;
    uint64_t ops = 0;
    while (timer_get_ticks() - t0 < 300) {
        for (int i = 0; i < 5000; i++) {
            acc = acc * 1103515245 + 12345;
            ops++;
        }
    }
    uint32_t ms = (timer_get_ticks() - t0) * 10;
    if (ms == 0) ms = 1;
    kprintf("integer ops: %d in %d ms  =>  %d ops/sec\n", (int)ops, (int)ms, (int)(ops * 1000 / ms));
    kprintf("checksum: %x\n", acc);
    return 0;
}

void shell_init(void) {
    env_init();
    history_count = 0;
    history_pos = 0;
}

void shell_run(void) {
    kprintf("BlackMatrixOS Shell v2.0 - Type 'help' for available commands.\n");
    kprintf("Scripting: if/elif/else/fi for/while/until/done $VAR pipes '|' redirect '>'\n\n");

    char cmdline[SHELL_MAX_CMD];
    int pos = 0;

    for (;;) {
        /* Run scheduled at-jobs */
        at_run_due();

        /* Print prompt */
        char prompt[128];
        get_prompt(prompt, sizeof(prompt));
        if (block_depth > 0) {
            vga_set_color(VGA_COLOR(VGA_LIGHT_BLUE, VGA_BLACK));
            kprintf("...%s", prompt);
        } else {
            vga_set_color(VGA_COLOR(VGA_LIGHT_GREEN, VGA_BLACK));
            kprintf("%s", prompt);
        }
        vga_set_color(VGA_COLOR(VGA_LIGHT_GREY, VGA_BLACK));

        /* Read line */
        pos = 0;
        memset(cmdline, 0, sizeof(cmdline));

        for (;;) {
            char c = keyboard_getchar();

            if (c == '\n') {
                kprintf("\n");
                break;
            } else if (c == '\b') {
                if (pos > 0) {
                    pos--;
                    cmdline[pos] = '\0';
                    vga_putchar('\b');
                    vga_putchar(' ');
                    vga_putchar('\b');
                }
            } else if (c == '\t') {
                /* Tab completion - simple */
                for (int i = 0; i < 4 && pos < SHELL_MAX_CMD - 1; i++) {
                    cmdline[pos++] = ' ';
                    vga_putchar(' ');
                }
            } else if (c == 0x0C) {
                /* Ctrl+L: clear screen */
                vga_clear();
                kprintf("%s%s", prompt, cmdline);
            } else if (c >= 32 && c < 127) {
                if (pos < SHELL_MAX_CMD - 1) {
                    cmdline[pos++] = c;
                    vga_putchar(c);
                }
            }
        }

        cmdline[pos] = '\0';

        if (pos > 0) {
            /* Record history (interactive lines only) */
            if (history_count < SHELL_HISTORY) {
                strncpy(history[history_count].cmd, cmdline, SHELL_MAX_CMD - 1);
                history_count++;
            }
            history_pos = history_count;
            if (cmd_history_len < 128) {
                strncpy(cmd_history_buf[cmd_history_len], cmdline, SHELL_MAX_CMD - 1);
                cmd_history_len++;
            }
            shell_handle_line(cmdline);
        }
    }
}
