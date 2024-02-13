#include "unity.h"
#include "fff.h"
#include "io_utility.h"
#include <string.h>

DEFINE_FFF_GLOBALS


void setUp(void){

}

void tearDown(void){

}


// fake read function for mocking
// intentionally doesn't read maximum. 
// can't read past predefined string
int fake_read1(int fd, void *buf, size_t size) {
    char s[] = "predefined_";
    strncpy(buf, s, strlen(s));
    return strlen(s);
}

int fake_read2(int fd, void *buf, size_t size) {
    char s[] = "string";
    strncpy(buf, s, strlen(s));
    return strlen(s);
}


FAKE_VALUE_FUNC(int, read, int, void*, size_t)

void test_readall(void){
    char expected[] = "predefined_string";
    char buf[128];
    size_t len = strlen(expected);

    int (*custom_fake[])(int, void*, size_t) = {fake_read1, fake_read2};
    SET_CUSTOM_FAKE_SEQ(read, custom_fake, 2);

    readall(1, buf, len);


    TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(buf, expected, len, "readall assertion");
}


int main(void){
    UNITY_BEGIN();
    RUN_TEST(test_readall);
    return UNITY_END();
}
