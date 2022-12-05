#ifdef __cplusplus
extern "C"{
#endif
    int convertImageDataToGrayScale(char *data, size_t* dataLen, char *);
    int convertImageDataToBlur(char *data, size_t* dataLen, char *);
    int convertImageDataToInverted(char *data, size_t* dataLen, char *);
    int convertImageDataToBorderedImage(char *data, size_t* dataLen, char *);

    int viewImage(char *data,size_t dataLen);
    int viewImageWithPath(char* path);
#ifdef __cplusplus
}
#endif