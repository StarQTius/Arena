include(CTest)

add_test_runner_target(check_coc)
add_python_test("The Cherry On The Cake" check_coc binding.py)

add_demo_runner_target("Demo" demo_coc demo.py)
