def bargraph_mid_from_level(level: int):
    start_value = 100
    end_value = 1010
    shift_a = 600
    shift_b = 710
    max_level = 10

    span_before = shift_a - start_value
    span_after = end_value - shift_b

    _, count_before, step_before, step_after = min(
        (
            (
                abs(span_before // count_before - span_after // (max_level - count_before)),
                count_before,
                span_before // count_before,
                span_after // (max_level - count_before),
            )
            for count_before in range(1, max_level)
            if span_before % count_before == 0 and span_after % (max_level - count_before) == 0
        ),
        key=lambda item: item[0],
    )
    if level < count_before:
        lo = start_value + level * step_before
        hi = lo + step_before
    elif level == count_before:
        lo, hi = shift_a, shift_b
    else:
        lo = shift_b + (level - count_before - 1) * step_after
        hi = lo + step_after
    return (lo + hi) // 2

level_values = [bargraph_mid_from_level(i) for i in range(0, 11)]

print("level -> value")
for i, v in enumerate(level_values):
    print(f"{i:>5} -> {v}")
