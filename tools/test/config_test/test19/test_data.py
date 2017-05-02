# Test that invalid keys in application configuration are not allowed

expected_results = {
    "K64F": {
        "desc": "test invalid key in application config",
        "exception_msg": "Additional properties are not allowed"
    }
}
