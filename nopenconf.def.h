/* See LICENSE file for copyright and license details. */
struct assoc assocs[] = {
	{ .regex = "\\.(avi|mp4|mkv|mp3|ogg|flac|mov)$", .file = "mpv", .argv = { "mpv", "{}", NULL } },
	{ .regex = "\\.(png|jpg|gif)$", .file = "sxiv", .argv = { "sxiv", "{}", NULL} },
	{ .regex = "\\.(html|svg)$", .file = "firefox", .argv = { "firefox", "{}", NULL } },
	{ .regex = "\\.pdf$", .file = "mupdf", .argv = { "mupdf", "{}", NULL} },
	{ .regex = "\\.sh$", .file = "sh", .argv = { "sh", "{}", NULL} },
	{ .regex = ".", .file = "less", .argv = { "less", "{}", NULL } },
};
