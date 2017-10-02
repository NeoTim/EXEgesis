# ARM XML instruction database parser.

package(default_visibility = ["//:internal_users"])

licenses(["notice"])  # Apache 2.0

proto_library(
    name = "parser_proto",
    srcs = [
        "docvars.proto",
        "parser.proto",
    ],
    deps = ["//exegesis/proto:instruction_encoding_proto"],
)

cc_proto_library(
    name = "parser_cc_proto",
    deps = [":parser_proto"],
)

cc_library(
    name = "converter",
    srcs = ["converter.cc"],
    hdrs = ["converter.h"],
    deps = [
        ":parser_cc_proto",
        "//base",
        "//exegesis/proto:instruction_encoding_cc_proto",
        "//exegesis/proto:instructions_cc_proto",
        "//strings",
        "//util/gtl:map_util",
        "//util/task:status",
        "//util/task:statusor",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_glog_glog//:glog",
        "@com_google_protobuf//:protobuf_lite",
    ],
)

cc_test(
    name = "converter_test",
    size = "small",
    srcs = ["converter_test.cc"],
    deps = [
        ":converter",
        ":parser_cc_proto",
        "//exegesis/proto:instructions_cc_proto",
        "//exegesis/testing:test_util",
        "//strings",
        "//util/task:status",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "docvars",
    srcs = ["docvars.cc"],
    hdrs = ["docvars.h"],
    copts = ["$(STACK_FRAME_UNLIMITED)"],
    deps = [
        ":parser_cc_proto",
        "//base",
        "//exegesis/util/xml:xml_util",
        "//strings",
        "//util/gtl:map_util",
        "//util/task:status",
        "//util/task:statusor",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_glog_glog//:glog",
        "@com_google_protobuf//:protobuf",
        "@com_google_protobuf//:protobuf_lite",
        "@tinyxml2_git//:tinyxml2",
    ],
)

cc_test(
    name = "docvars_test",
    size = "small",
    srcs = ["docvars_test.cc"],
    deps = [
        ":docvars",
        ":parser_cc_proto",
        "//exegesis/testing:test_util",
        "//util/task:status",
        "@com_google_googletest//:gtest_main",
        "@tinyxml2_git//:tinyxml2",
    ],
)

cc_library(
    name = "markdown",
    srcs = ["markdown.cc"],
    hdrs = ["markdown.h"],
    deps = [
        "//base",
        "//exegesis/util/xml:xml_util",
        "//strings",
        "//util/gtl:map_util",
        "//util/task:status",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_glog_glog//:glog",
        "@com_google_protobuf//:protobuf_lite",
        "@com_googlesource_code_re2//:re2",
        "@tinyxml2_git//:tinyxml2",
    ],
)

cc_test(
    name = "markdown_test",
    size = "small",
    srcs = ["markdown_test.cc"],
    deps = [
        ":markdown",
        "@com_google_googletest//:gtest_main",
        "@tinyxml2_git//:tinyxml2",
    ],
)

cc_library(
    name = "parser",
    srcs = ["parser.cc"],
    hdrs = ["parser.h"],
    deps = [
        ":docvars",
        ":markdown",
        ":parser_cc_proto",
        "//base",
        "//exegesis/proto:instruction_encoding_cc_proto",
        "//exegesis/util/xml:xml_util",
        "//file/base:path",
        "//net/proto2/util/public:repeated_field_util",
        "//strings",
        "//util/task:status",
        "//util/task:statusor",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_glog_glog//:glog",
        "@com_google_protobuf//:protobuf_lite",
        "@tinyxml2_git//:tinyxml2",
    ],
)

cc_test(
    name = "parser_test",
    size = "small",
    srcs = ["parser_test.cc"],
    data = [
        "testdata/fpsimdindex.xml",
        "testdata/index.xml",
        "testdata/instruction_1.xml",
        "testdata/instruction_2.xml",
    ],
    deps = [
        ":parser",
        "//exegesis/testing:test_util",
        "//strings",
        "//util/task:status",
        "@com_google_googletest//:gtest_main",
    ],
)