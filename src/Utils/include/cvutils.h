#ifdef __cplusplus
extern "C"{
#endif
    int convertImageDataToGrayScale(char *data, ssize_t* dataLen);
    int viewImage(char *data,ssize_t dataLen);
    int viewImageWithPath(char* path);
#ifdef __cplusplus
}
#endif