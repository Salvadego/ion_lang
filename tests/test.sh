#!/usr/bin/env bash

#
# Global variables only
#

exec_errors=()
diff_failures=()

suite="$1"
suite_dir="tests/$suite"
bin="./bin/test-$suite"

is_ci=0
if [[ "${CI:-}" == "true" ]]; then
	is_ci=1
fi

timeout_cmd=''


#
# Color + formatted output helpers
#

_fmt() {
    local text="$2"
	local code="$1"

	if (( is_ci == 1 )); then
		printf '%s' "$text"
		return
	fi

	printf '\033[%sm%s\033[0m' "$code" "$text"
}

_green() { _fmt '32' "$1"; }
_red() { _fmt '31' "$1"; }
_blue() { _fmt '34' "$1"; }
_bold() { _fmt '1' "$1"; }

info() {
	local msg="$1"
	printf '%s %s\n' "$(_blue '[INFO]')" "$msg"
}

success() {
	local msg="$1"
	printf '%s %s\n' "$(_green '[OK]')" "$msg"
}

warn() {
	local msg="$1"
	printf '%s %s\n' "$(_fmt '33' '[WARN]')" "$msg"
}

error() {
	local msg="$1"
	printf '%s %s\n' "$(_red '[ERR]')" "$msg"
}

die() {
	local msg="$1"
	error "$msg"
	exit 1
}


#
# Determine timeout command
#

detect_timeout() {
	if command -v timeout >/dev/null 2>&1; then
		timeout_cmd='timeout'
		return
	fi

	if command -v gtimeout >/dev/null 2>&1; then
		timeout_cmd='gtimeout'
		return
	fi

	die "timeout or gtimeout not found"
}


#
# Run all test cases in parallel
#

run_tests() {
	local -n out_pids=$1
	local -n out_files=$2

	for path in "$suite_dir"/*.ion; do
		# pattern with no matches → skip
		if [[ ! -e "$path" ]]; then
			continue
		fi

		local name="${path##*/}"
		name="${name%.ion}"

		local out_file="$suite_dir/$name.out"

		"$timeout_cmd" 10s "$bin" "$path" >"$out_file" 2>&1 &
		out_pids+=("$!")
		out_files+=("$name")
	done
}


#
# Wait for background jobs and record runtime errors
#

collect_exec_errors() {
	local -n _pids=$1
	local -n _files=$2

	for idx in "${!_pids[@]}"; do
		local pid="${_pids[$idx]}"
		local name="${_files[$idx]}"

		if ! wait "$pid"; then
			local out_file="$suite_dir/$name.out"
			printf '%s\n' "[FAILED]" >>"$out_file"
			exec_errors+=("$name")
		fi
	done
}


#
# Diff output vs expected
#

diff_outputs() {
	local -n _files=$1
	local -n _passed=$2

	for name in "${_files[@]}"; do
		# skip if runtime error
		for bad in "${exec_errors[@]}"; do
			if [[ "$bad" == "$name" ]]; then
				continue 2
			fi
		done

		local out_file="$suite_dir/$name.out"
		local exp_file="$suite_dir/$name.ans"
		local diff_file="$suite_dir/$name.diff"

		if ! diff -u "$exp_file" "$out_file" >"$diff_file"; then
			diff_failures+=("$name")
		else
			rm -f "$diff_file"
			((_passed+=1))
		fi
	done
}


#
# Print test summary
#

print_summary() {
	local passed="$1"

	printf '\n'
	printf '%s\n' "$(_green "$(_bold '******** SUMMARY ********')")"
	printf '\n'

	if (( ${#exec_errors[@]} > 0 )); then
		error "Execution errors:"
		for name in "${exec_errors[@]}"; do
			printf '\t- %s\n' "$name"
		done
		printf '\n'
	fi

	if (( ${#diff_failures[@]} > 0 )); then
		error "Output mismatches:"
		for name in "${diff_failures[@]}"; do
			printf '\t- %s\n' "$name"
		done
		printf '\n'
	fi

	if (( ${#exec_errors[@]} == 0 && ${#diff_failures[@]} == 0 )); then
		success "All tests passed ($passed)"
		return 0
	fi

	info "$passed passed"
	return 1
}


#
# Main
#

main() {
	[[ -n "$suite" ]] || die "missing suite name (usage: ./test.sh <suite>)"
	[[ -x "$bin" ]] || die "test binary missing: $bin"
	[[ -d "$suite_dir" ]] || die "suite folder missing: $suite_dir"

	detect_timeout

	local pids=()
	local files=()
	local passed=0

	run_tests pids files
	collect_exec_errors pids files
	diff_outputs files passed

	print_summary "$passed"
}


main "$@"
