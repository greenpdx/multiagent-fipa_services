#include <boost/test/unit_test.hpp>
#include <iostream>
#include <sstream>
#include <fipa_services/DistributedServiceDirectory.hpp>
BOOST_AUTO_TEST_SUITE(distributed_service_directory_suite)

std::string getProtocolPath()
{
    char buffer[1024];
    BOOST_REQUIRE_MESSAGE( readlink("/proc/self/exe", buffer, 1024) != -1, "Retrieving current execution path");
    std::string str(buffer);
    std::string executionDir = str.substr(0, str.rfind('/'));
    // Assuming we have do a build into build/ parallel to src/ 
    std::string configurationPath = executionDir + "/../../../../configuration/protocols";
    return configurationPath;
}

BOOST_AUTO_TEST_CASE(distributed_service_directory_test)
{
    using namespace fipa::services;
    using namespace servicediscovery;

    Name name("test-name");
    Type type = "_test._tcp";
    ServiceLocator locator = ServiceLocator::fromString("url://test/url;http://bla.org");
    Description description = "test description of this service";

    ServiceDirectoryEntry entry(name, type, locator, description);
    servicediscovery::avahi::ServiceDescription serviceDescription = DistributedServiceDirectory::convert(entry);

    {
        std::string expected = name;
        std::string received = serviceDescription.getName();

        BOOST_REQUIRE_MESSAGE(expected == received, "Distributed: conversion name -- got '" << received << "' expected '" << expected << "'");
    }
    {
        std::string expected = type;
        std::string received = serviceDescription.getDescription(ServiceDirectoryEntry::FieldTxt[ServiceDirectoryEntry::TYPE]);

        BOOST_REQUIRE_MESSAGE(expected == received, "Distributed: conversion type -- got '" << received << "' expected '" << expected << "'");
    }
    {
        std::string expected = locator.toString();
        std::string received = serviceDescription.getDescription(ServiceDirectoryEntry::FieldTxt[ServiceDirectoryEntry::LOCATOR]);

        BOOST_REQUIRE_MESSAGE(expected == received, "Distributed: conversion locator -- got '" << received << "' expected '" << expected << "'");
    }
    {
        std::string expected = description;
        std::string received = serviceDescription.getDescription(ServiceDirectoryEntry::FieldTxt[ServiceDirectoryEntry::DESCRIPTION]);

        BOOST_REQUIRE_MESSAGE(expected == received, "Distributed: conversion description -- got '" << received << "' expected '" << expected << "'");
    }

    DistributedServiceDirectory distributedServiceDirectory;
    BOOST_REQUIRE_THROW( distributedServiceDirectory.search(name, ServiceDirectoryEntry::NAME, true), NotFound );
    // Register 5 Services
    for(int i = 0; i < 5; ++i)
    {
        std::stringstream ss;
        ss << "base_" << i;
        ServiceDirectoryEntry entry(ss.str(), type, locator, description);
        distributedServiceDirectory.registerService(entry);
    }
    sleep(5);
    
    // Test they're registered
    for(int i = 0; i < 5; ++i)
    {
        std::stringstream ss;
        ss << "base_" << i;
        BOOST_REQUIRE_NO_THROW( distributedServiceDirectory.search(ss.str(), ServiceDirectoryEntry::NAME, true));
    }
    
    // Deregister them again
    for(int i = 0; i < 5; ++i)
    {
        std::stringstream ss;
        ss << "base_" << i;
        distributedServiceDirectory.deregisterService(ss.str(), ServiceDirectoryEntry::NAME);
    }
    sleep(10);
    
    // Test they're deregistered
    for(int i = 0; i < 5; ++i)
    {
        std::stringstream ss;
        ss << "base_" << i;
        BOOST_REQUIRE_THROW( distributedServiceDirectory.search(ss.str(), ServiceDirectoryEntry::NAME, true), NotFound);
    }
}


BOOST_AUTO_TEST_SUITE_END()
