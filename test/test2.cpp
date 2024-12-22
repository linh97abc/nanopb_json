#include <pb/json.h>
#include <test_json.pb.h>
#include <iostream>
#include <string.h>

void test1()
{
    char s[256];

    SubMessage1 msg1 = SubMessage1_init_default;

    msg1.array_count = 2;
    msg1.array[0] = 12;
    msg1.array[1] = 14;

    int len = pbjson_encode(s, sizeof(s), SubMessage1_fields, &msg1);

    if (len < 0)
    {
        std::cout << "encode error" << std::endl;
        return;
    }

    std::cout << s << std::endl;
}

void test2()
{
    char s[256];

    SubMessage2 msg = SubMessage2_init_default;

    msg.x = 1.23f;
    msg.y = -12;

    int len = pbjson_encode(s, sizeof(s), SubMessage2_fields, &msg);

    if (len < 0)
    {
        std::cout << "encode error" << std::endl;
        return;
    }

    std::cout << s << std::endl;
}

void test3()
{
    char s[256];

    SubMessage3 msg = SubMessage3_init_default;

    msg.opt = TestEnum::TestEnum_Opt2;
    msg.has_msg = true;
    msg.msg.x = 1.27f;
    msg.msg.y = -25;
    
    snprintf(msg.x, sizeof(msg.x), "Hello");

    int len = pbjson_encode(s, sizeof(s), SubMessage3_fields, &msg);

    if (len < 0)
    {
        std::cout << "encode error" << std::endl;
        // return;
    }

    std::cout << s << std::endl;
}

void test3_1()
{
    char s[256];

    SubMessage3 msg = SubMessage3_init_default;

    msg.opt = TestEnum::TestEnum_Opt1;
    msg.has_msg = false;

    snprintf(msg.x, sizeof(msg.x), "AH-sd");

    int len = pbjson_encode(s, sizeof(s), SubMessage3_fields, &msg);

    if (len < 0)
    {
        std::cout << "encode error" << std::endl;
        return;
    }

    std::cout << s << std::endl;
}

void test4()
{
    char s[1024];

    SubMessage4 msg = SubMessage4_init_zero;

    int len = pbjson_encode(s, sizeof(s), SubMessage4_fields, &msg);

    if (len < 0)
    {
        std::cout << "encode error" << std::endl;
        return;
    }

    std::cout << s << std::endl;
}

void test5()
{
    char s[1024];

    SubMessage5 msg = SubMessage5_init_zero;

    msg.s_count = 3;
    snprintf(msg.s[0], sizeof(msg.s[0]), "msg1");
    snprintf(msg.s[1], sizeof(msg.s[1]), "msg2");
    snprintf(msg.s[2], sizeof(msg.s[2]), "msg3");

    int len = pbjson_encode(s, sizeof(s), SubMessage5_fields, &msg);

    if (len < 0)
    {
        std::cout << "encode error" << std::endl;
        return;
    }

    std::cout << s << std::endl;
}

void test6()
{
    char s[1024];

    SubMessage6 msg = SubMessage6_init_zero;

    msg.x_count = 3;
    msg.x[0].x = 1;
    msg.x[0].y = 2;

    msg.x[1].x = 14;
    msg.x[1].y = 21;

    msg.x[2].x = 17;
    msg.x[2].y = 29;

    int len = pbjson_encode(s, sizeof(s), SubMessage6_fields, &msg);

    if (len < 0)
    {
        std::cout << "encode error" << std::endl;
        return;
    }

    std::cout << s << std::endl;
}

void test7()
{
    char s[1024];

    SubMessage7 msg = SubMessage7_init_zero;

    msg.has_x = true;
    msg.has_y = true;

    SubMessage2 &msg2 = msg.x;

    msg2.x = 1.23f;
    msg2.y = -12;

    auto &msg3 = msg.y;

    msg3.opt = TestEnum::TestEnum_Opt2;
    msg3.has_msg = true;
    msg3.msg.x = 1.27f;
    msg3.msg.y = -25;
    
    snprintf(msg3.x, sizeof(msg3.x), "Hello");

    int len = pbjson_encode(s, sizeof(s), SubMessage7_fields, &msg);

    if (len < 0)
    {
        std::cout << "encode error" << std::endl;
        return;
    }

    std::cout << s << std::endl;
}

void test_decode1()
{
    char s[256];

    SubMessage2 msg = SubMessage2_init_default;

    msg.x = 1.23f;
    msg.y = -12;

    int len = pbjson_encode(s, sizeof(s), SubMessage2_fields, &msg);

    SubMessage2 rmsg = SubMessage2_init_default;

    int err = pbjson_decode(s, SubMessage2_fields, &rmsg);

    if (err)
    {
        std::cout << "decode error" << std::endl;
        return;
    }
}

void test_decode2()
{
    char s[1024];

    SubMessage7 msg = SubMessage7_init_zero;

    msg.has_x = true;
    msg.has_y = true;

    SubMessage2 &msg2 = msg.x;

    msg2.x = 1.23f;
    msg2.y = -12;

    auto &msg3 = msg.y;

    msg3.opt = TestEnum::TestEnum_Opt2;
    msg3.has_msg = true;
    msg3.msg.x = 1.27f;
    msg3.msg.y = -25;
    // strncpy(msg3.x, "Hello", sizeof(msg3.x) - 1);
    snprintf(msg3.x, sizeof(msg3.x), "Hello");

    int len = pbjson_encode(s, sizeof(s), SubMessage7_fields, &msg);

    SubMessage7 rmsg = SubMessage7_init_zero;

    int err = pbjson_decode(s, SubMessage7_fields, &rmsg);

    if (err)
    {
        std::cout << "decode error" << std::endl;
        return;
    }
}

SubMessage6 *pmsg6;

void test_decode3()
{
    char s[1024];

    SubMessage6 msg = SubMessage6_init_zero;

    msg.x_count = 3;
    msg.x[0].x = 1;
    msg.x[0].y = 2;

    msg.x[1].x = 14;
    msg.x[1].y = 21;

    msg.x[2].x = 17;
    msg.x[2].y = 29;

    int len = pbjson_encode(s, sizeof(s), SubMessage6_fields, &msg);

    SubMessage6 rmsg = SubMessage6_init_zero;
    pmsg6 = &rmsg;

    int err = pbjson_decode(s, SubMessage6_fields, &rmsg);

    if (err)
    {
        std::cout << "decode error" << std::endl;
        return;
    }
}

SubMessage1 *pmsg1;

void test_decode4()
{
    char s[256];

    SubMessage1 msg1 = SubMessage1_init_default;

    msg1.array_count = 2;
    msg1.array[0] = 12;
    msg1.array[1] = 14;

    int len = pbjson_encode(s, sizeof(s), SubMessage1_fields, &msg1);

    SubMessage1 rmsg = SubMessage1_init_default;
    pmsg1 = &rmsg;

    int err = pbjson_decode(s, SubMessage1_fields, &rmsg);

    if (err)
    {
        std::cout << "decode error" << std::endl;
        return;
    }
}

void test_decode5()
{
    char s[1024];

    SubMessage5 msg = SubMessage5_init_zero;

    msg.s_count = 3;
    snprintf(msg.s[0], sizeof(msg.s[0]), "msg1");
    snprintf(msg.s[1], sizeof(msg.s[1]), "msg2");
    snprintf(msg.s[2], sizeof(msg.s[2]), "msg3");

    int len = pbjson_encode(s, sizeof(s), SubMessage5_fields, &msg);

    SubMessage5 rmsg = SubMessage5_init_zero;

    int err = pbjson_decode(s, SubMessage5_fields, &rmsg);

    if (err)
    {
        std::cout << "decode error" << std::endl;
        return;
    }
}

void test_decode6()
{
    SubMessage1 msg = SubMessage1_init_default;
    const char *s = "{  \"array\"  : [] } ";

    int err = pbjson_decode(s, SubMessage1_fields, &msg);

    if (err)
    {
        std::cout << "decode error" << std::endl;
    }
}

void test_decode7()
{
    SubMessage7 msg = SubMessage7_init_zero;

    const char *s = "{\"x\" : {} , \"y\":{\"x\":\"Hello\",\"msg\":{\"x\":1.270000,\"y\":-25},\"opt\":2}}";

    int err = pbjson_decode(s, SubMessage7_fields, &msg);

    if (err)
    {
        std::cout << "decode error" << std::endl;
    }
}

int main()
{
    test1();
    test2();
    test3();
    test3_1();
    test4();
    test5();

    test6();
    test7();

    test_decode1();
    test_decode2();
    test_decode3();
    test_decode4();
    test_decode5();
    test_decode6();
    test_decode7();

    return 0;
}