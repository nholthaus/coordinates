#!/usr/bin/env bash
# ---------------------------------------------------------------------------------------------------------------------
# Render the f35Attitude example's seamless-loop frames and encode them into a looping MP4 and GIF.
#
# Usage:  scripts/f35_video.sh [path/to/f35Attitude] [frameCount] [outputStem]
#   f35Attitude   path to the built example binary (default: the first match under build*/examples)
#   frameCount    frames spanning one loop (default: 120)
#   outputStem    output basename; writes <stem>.mp4 and <stem>.gif (default: f35_loop)
#
# Requires ffmpeg on PATH. The frames are written to a temporary directory and removed afterward; only the
# encoded MP4/GIF remain, so nothing binary needs to live in the repository.
# ---------------------------------------------------------------------------------------------------------------------
set -euo pipefail

binary="${1:-$(find build* -maxdepth 3 -type f -name f35Attitude -perm -u+x 2>/dev/null | head -1)}"
frames="${2:-120}"
stem="${3:-f35_loop}"

if [[ -z "${binary}" || ! -x "${binary}" ]]; then
	echo "f35Attitude binary not found; build the example first (or pass its path)." >&2
	exit 1
fi
command -v ffmpeg >/dev/null || { echo "ffmpeg not found on PATH." >&2; exit 1; }

tmp="$(mktemp -d)"
trap 'rm -rf "${tmp}"' EXIT

"${binary}" "${tmp}" "${frames}"

ffmpeg -y -framerate 30 -i "${tmp}/frame_%04d.ppm" -c:v libx264 -pix_fmt yuv420p -movflags +faststart "${stem}.mp4"
ffmpeg -y -framerate 30 -i "${tmp}/frame_%04d.ppm" -vf "fps=25,scale=480:-1:flags=lanczos" "${stem}.gif"

echo "wrote ${stem}.mp4 and ${stem}.gif"
