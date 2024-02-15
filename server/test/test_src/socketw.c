#include "fff.h"
#include "unity.h"
#include "socketw.h"

DEFINE_FFF_GLOBALS

SocketW *socketw;
char s[] = "what four word sand\n";
char s1[] = "random message that is random\n";
char s2[] = "another random that is not random\n";
char s3[] = "a longer message that is not longer\n";

void setUp(void) {
    socketw = NULL;
    socketw = socketw_create();
}

void tearDown(void) { socketw_destroy(&socketw); }

void test_socketw_create_destroy() {
    // given
    SocketW *socketw = NULL;

    // when
    socketw = socketw_create();

    // then
    TEST_ASSERT_NOT_NULL_MESSAGE(socketw, "Socket is NULL");

    // when
    socketw_destroy(&socketw);

    // then
    TEST_ASSERT_NULL_MESSAGE(socketw, "Socket is not NULL");
}

void test_socketw_create_destroy_msg() { 
    const int len = 8;
    SocketWMsg *msg = socketw_create_msg(1, s, len);

    TEST_ASSERT_NOT_NULL_MESSAGE(msg, "msg is null");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, msg->client_num, "client_num is different");
    TEST_ASSERT_EQUAL_size_t_MESSAGE(len, msg->msg_len, "msg_len is different");
    TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(s, msg->msg, len, "msg string is different");
    
    socketw_destroy_msg(&msg);
    TEST_ASSERT_NULL_MESSAGE(msg, "msg is not null after free");
}

void test_socketw_enqueue_single() {
    // given
    SocketWMsg *msg1 = socketw_create_msg(1, s1, 5);

    // when
    socketw_enqueue(socketw, msg1);
    
    // then
    TEST_ASSERT_EQUAL_size_t_MESSAGE(1, socketw_queue_len(socketw), "queue size is not consistent in enqueue");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(msg1, socketw_queue_front(socketw), "queue front is not correct");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(msg1, socketw_queue_back(socketw), "queue back is not correct");
}

void test_socketw_enqueue_multiple() {
    // given
    SocketWMsg *msg1 = socketw_create_msg(1, s1, 5);
    SocketWMsg *msg2 = socketw_create_msg(3, s2, 8);
    SocketWMsg *msg3 = socketw_create_msg(5, s3, 10);

    // when
    socketw_enqueue(socketw, msg1);
    socketw_enqueue(socketw, msg2);
    socketw_enqueue(socketw, msg3);

    // then
    TEST_ASSERT_EQUAL_size_t_MESSAGE(3, socketw_queue_len(socketw), "queue size is not consistent in enqueue");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(msg1, socketw_queue_front(socketw), "queue front is not correct");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(msg3, socketw_queue_back(socketw), "queue back is not correct");
}

void test_socketw_enqueue_too_much() {
    int res;
    for(int i=0; i<SOCKETW_QUEUE_SIZE; i++){
        res = socketw_enqueue(socketw, socketw_create_msg(1, s1, 5));
    }

    TEST_ASSERT_EQUAL_INT(-1, res);
}

void test_socketw_dequeue_nothing() {
    // given
    //
    // when
    SocketWMsg *msg = socketw_dequeue(socketw);

    // then
    TEST_ASSERT_NULL_MESSAGE(msg, "msg not null");
}

void test_socketw_dequeue_multiple() {
    // given
    SocketWMsg *msg1 = socketw_create_msg(1, s1, 5);
    SocketWMsg *msg2 = socketw_create_msg(3, s2, 8);
    SocketWMsg *msg3 = socketw_create_msg(5, s3, 10);

    // when
    socketw_enqueue(socketw, msg1);
    socketw_enqueue(socketw, msg2);
    socketw_enqueue(socketw, msg3);

    SocketWMsg *dmsg1 = socketw_dequeue(socketw);
    SocketWMsg *dmsg2 = socketw_dequeue(socketw);
    SocketWMsg *dmsg3 = socketw_dequeue(socketw);

    // then
    TEST_ASSERT_EQUAL_PTR_MESSAGE(msg1, dmsg1, "wrong thing dequeued");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(msg2, dmsg2, "wrong thing dequeued");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(msg3, dmsg3, "wrong thing dequeued");
    
}


void test_socketw_run_server() {

}

void test_socketw_send_msgs(){

}

void test_github_action() {
    TEST_FAIL();
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_socketw_create_destroy);
    RUN_TEST(test_socketw_create_destroy_msg);
    // queue
    RUN_TEST(test_socketw_enqueue_single);
    RUN_TEST(test_socketw_enqueue_multiple);
    RUN_TEST(test_socketw_enqueue_too_much);
    RUN_TEST(test_socketw_dequeue_nothing);
    RUN_TEST(test_socketw_dequeue_multiple);

    RUN_TEST(test_github_action);

    return UNITY_END();
}

/*
* TEST LIST
* 
* 
*/
