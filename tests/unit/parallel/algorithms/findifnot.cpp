//  copyright (c) 2014 Grant Mercer
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx_init.hpp>
#include <hpx/hpx.hpp>
#include <hpx/include/parallel_find.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <boost/range/functions.hpp>

#include <string>
#include <vector>

#include "test_utils.hpp"

////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_find_if_not(ExPolicy policy, IteratorTag)
{
    static_assert(
        hpx::parallel::is_execution_policy<ExPolicy>::value,
        "hpx::parallel::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c(10007);
    //fill vector with random values about 1
    std::fill(boost::begin(c), boost::end(c), (std::rand()%100)+2);
    c.at(c.size()/2) = 1;

    iterator index = hpx::parallel::find_if_not(policy,
        iterator(boost::begin(c)), iterator(boost::end(c)),
        [](std::size_t v) {
            return v != std::size_t(1);
        });

    base_iterator test_index = boost::begin(c) + c.size()/2;

    HPX_TEST(index == iterator(test_index));
}

template <typename ExPolicy, typename IteratorTag>
void test_find_if_not_async(ExPolicy p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c(10007);
    //fill vector with random values above 1
    std::fill(boost::begin(c), boost::end(c), (std::rand()%100) + 2);
    c.at(c.size()/2) = 1;

    hpx::future<iterator> f =
        hpx::parallel::find_if_not(p,
            iterator(boost::begin(c)), iterator(boost::end(c)),
        [](std::size_t v) {
            return v != std::size_t(1);
        });
    f.wait();

    //create iterator at position of value to be found
    base_iterator test_index = boost::begin(c) + c.size()/2;

    HPX_TEST(f.get() == iterator(test_index));
}

template <typename IteratorTag>
void test_find_if_not()
{
    using namespace hpx::parallel;
    test_find_if_not(seq, IteratorTag());
    test_find_if_not(par, IteratorTag());
    test_find_if_not(par_vec, IteratorTag());

    test_find_if_not_async(seq(task), IteratorTag());
    test_find_if_not_async(par(task), IteratorTag());

#if defined(HPX_HAVE_GENERIC_EXECUTION_POLICY)
    test_find_if_not(execution_policy(seq), IteratorTag());
    test_find_if_not(execution_policy(par), IteratorTag());
    test_find_if_not(execution_policy(par_vec), IteratorTag());

    test_find_if_not(execution_policy(seq(task)), IteratorTag());
    test_find_if_not(execution_policy(par(task)), IteratorTag());
#endif
}

void find_if_not_test()
{
    test_find_if_not<std::random_access_iterator_tag>();
    test_find_if_not<std::forward_iterator_tag>();
    test_find_if_not<std::input_iterator_tag>();
}

int hpx_main(boost::program_options::variables_map& vm)
{
    unsigned int seed = (unsigned int)std::time(0);
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    std::srand(seed);

    find_if_not_test();
    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // add command line option which controls the random number generator seed
    using namespace boost::program_options;
    options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");

    desc_commandline.add_options()
        ("seed,s", value<unsigned int>(),
        "the random number generator seed to use for this run")
        ;

    // By default this test should run on all available cores
    std::vector<std::string> cfg;
    cfg.push_back("hpx.os_threads=" +
        std::to_string(hpx::threads::hardware_concurrency()));

    // Initialize and run HPX
    HPX_TEST_EQ_MSG(hpx::init(desc_commandline, argc, argv, cfg), 0,
        "HPX main exited with non-zero status");

    return hpx::util::report_errors();
}
