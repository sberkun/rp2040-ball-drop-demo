import sys
import re

import statistics

A_RISE = r"#(\d+) 1\(\n"
A_FALL = r"#(\d+) 0\(\n"
B_RISE = r"#(\d+) 1&\n"
B_FALL = r"#(\d+) 0&\n"



def diff(lst):
    return [lst[a] - lst[a-1] for a in range(1, len(lst)-1)]


def diff_bet(l1, l2):
    while min(len(l1), len(l2)) > 0 and l2[0] < l1[0]:
        l2 = l2[1:]
    return [l2[a] - l1[a] for a in range(min(len(l1), len(l2)))]


def safe_mean(lst):
    return 0.0 if len(lst) == 0 else statistics.mean(lst)

def safe_stdev(lst):
    return 0.0 if len(lst) < 2 else statistics.stdev(lst)

def print_stats(la, lb):
    print("  A mean:", safe_mean(la))
    print("  B mean:", safe_mean(lb))
    print("    mean:", safe_mean(la + lb))
    print("  A stdv:", safe_stdev(la))
    print("  B stdv:", safe_stdev(lb))
    print("    stdv:", safe_stdev(la + lb))


def extract_numbers(filename, rex):
    numbers = []
    with open(filename, 'r') as file:
        for line in file:
            match = re.match(rex, line)
            if match:
                number = int(match.group(1)) / 10000
                numbers.append(number)
    return numbers

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Please enter filename")
        sys.exit(1)
    
    filename = sys.argv[1]
    a_rise_times = extract_numbers(filename, A_RISE)
    a_fall_times = extract_numbers(filename, A_FALL)
    b_rise_times = extract_numbers(filename, B_RISE)
    b_fall_times = extract_numbers(filename, B_FALL)

    print("rise differences:")
    print_stats(diff(a_rise_times), diff(b_rise_times))

    print("high times:")
    print_stats(diff_bet(a_rise_times, a_fall_times), diff_bet(b_rise_times, b_fall_times))

    print("low times:")
    print_stats(diff_bet(a_fall_times, a_rise_times), diff_bet(b_fall_times, b_rise_times))
