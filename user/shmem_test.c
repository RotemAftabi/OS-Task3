  #include "kernel/types.h"
  #include "kernel/stat.h"
  #include "user/user.h"

  #define SHARED_SIZE 4096 //4KB

  int
  main(int argc, char *argv[])
  {
    int do_unmap = 1;
    if (argc > 1 && strcmp(argv[1], "no-unmap") == 0)
      do_unmap = 0;


    char *src = malloc(SHARED_SIZE);
    if (!src) {
      printf("Parent: malloc failed\n");
      exit(1);
    }

    int parent_pid = getpid();

    int pid = fork();
    if (pid < 0) {
      printf("Fork failed\n");
      exit(1);
    }

    if (pid == 0) {
      // Child process
      printf("Child: size before mapping = %d\n", sbrk(0));
      char *shared = (char *)map_shared_pages(parent_pid,(uint64) src, SHARED_SIZE);
      if (shared == (char *)0) {
        printf("Child: mapping failed\n");
        exit(1);
      }

      printf("Child: size after mapping = %d\n", sbrk(0));

      strcpy(shared, "Hello daddy");

      if (do_unmap) {
        if (unmap_shared_pages((uint64)shared, SHARED_SIZE) < 0) {
          printf("Child: unmap failed\n");
          exit(1);
        }
        printf("Child: size after unmapping = %d\n", sbrk(0));
      }

      void *new_mem = malloc(100);
      if (new_mem)
        printf("Child: malloc successful after unmapping\n");
      else
        printf("Child: malloc failed\n");

      printf("Child: final size = %d\n", sbrk(0));
      exit(0);

    } else {
      wait(0); // Wait for child

      printf("Parent: read string = %s\n", src);

      if (do_unmap) {
        if (unmap_shared_pages((uint64)src, SHARED_SIZE) < 0) {
          printf("Parent: unmap failed\n");
          exit(1);
        }
      }

      exit(0);
    }
  }
