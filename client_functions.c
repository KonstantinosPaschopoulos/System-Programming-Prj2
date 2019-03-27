// 
//
// void new_id(char *file_name){
//   pid_t sender, receiver;
//
//   if(strstr(file_name, ".id") != NULL)
//   {
//     // The new file that was created is not a .id file
//     return;
//   }
//
//   sender = fork();
//   if (sender < 0)
//   {
//     perror("Sender Fork Failed");
//     exit(2);
//   }
//
//   if (sender == 0)
//   {
//     // skew(atoi(argv[7]), atoi(argv[6]), atoi(argv[11]), atoi(argv[4]), &search_start, &search_end);
//     // sprintf(startStr, "%d", search_start);
//     // sprintf(endStr, "%d", search_end);
//     //
//     // execl("leaf", "leaf", argv[1], startStr, endStr, argv[2], rl_pipe, argv[9], NULL);
//     // exit(0);
//   }
//
//   receiver = fork();
//   if (receiver < 0)
//   {
//     perror("Receiver Fork Failed");
//     exit(2);
//   }
//
//   if (receiver == 0)
//   {
//     // skew(atoi(argv[7]), atoi(argv[6]), atoi(argv[11]), atoi(argv[4]), &search_start, &search_end);
//     // sprintf(startStr, "%d", search_start);
//     // sprintf(endStr, "%d", search_end);
//     //
//     // execl("leaf", "leaf", argv[1], startStr, endStr, argv[2], rl_pipe, argv[9], NULL);
//     // exit(0);
//   }
//
//
// }
