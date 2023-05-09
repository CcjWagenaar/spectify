






char super_secret = 97;

void check_passwd(int user_idx, char user_char, char user_passwd){
    char secret = 'x';
    char buf[10];

    //bounds check should prevent overwrite. But speculatively executes
    if (user_idx < 10)
        buf[user_idx] = user_char;

    //secret has been (speculatively) overwritten with user char.
    if (user_passwd == secret)
        printf("super secret: %c\n", reload_buffer[super_secret]);
}


int main(int argc, char** argv) {

    //check_passwd(11, 's', 's');

    //reload();

    //print_results
}