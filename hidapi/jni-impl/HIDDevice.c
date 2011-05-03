#include <stdio.h>
#include <assert.h>

#include <jni-stubs/com_codeminders_hidapi_HIDDevice.h>
#include "hidapi/hidapi.h"
#include "hid-java.h"

#define MAX_BUFFER_SIZE 2014

static hid_device* getPeer(JNIEnv *env, jobject self)
{
    jclass cls = (*env)->FindClass(env, DEV_CLASS);
    assert(cls!=NULL);
    if (cls == NULL) 
        return NULL;
    jfieldID fid = (*env)->GetFieldID(env, cls, "peer", "J");
    return (hid_device*)((*env)->GetLongField(env, self, fid));
}

static void setPeer(JNIEnv *env, jobject self, hid_device *peer)
{
    jclass cls = (*env)->FindClass(env, DEV_CLASS);
    assert(cls!=NULL);
    if (cls == NULL) 
        return; //TODO: exception will be raised by FindClass
    jfieldID fid = (*env)->GetFieldID(env, cls, "peer", "J");
    jlong peerj = (jlong)peer;
    (*env)->SetLongField(env, self, fid, peerj);     
}

JNIEXPORT void JNICALL Java_com_codeminders_hidapi_HIDDevice_close
  (JNIEnv *env, jobject self)
{
    hid_device *peer = getPeer(env, self);
    if(!peer) 
    {
        throwIOException(env, peer);
        return; /* not an error, freed previously */ 
    }
    hid_close(peer);
    setPeer(env, self, NULL);
}

JNIEXPORT jint JNICALL Java_com_codeminders_hidapi_HIDDevice_write
  (JNIEnv *env, jobject self, jbyteArray data)
{
    hid_device *peer = getPeer(env, self);
    if(!peer) 
    {
        throwIOException(env, peer);
        return; /* not an error, freed previously */ 
    }

    jsize bufsize = (*env)->GetArrayLength(env, data);
    jbyte *buf = (*env)->GetByteArrayElements(env, data, NULL);
    int res = hid_write(peer, buf, bufsize);
    (*env)->ReleaseByteArrayElements(env, data, buf, JNI_ABORT);
    if(res==-1)
    {
        throwIOException(env, peer);
        return; /* not an error, freed previously */ 
    }
    return res;
}

JNIEXPORT jint JNICALL Java_com_codeminders_hidapi_HIDDevice_read
  (JNIEnv *env, jobject self, jbyteArray data)
{
    hid_device *peer = getPeer(env, self);
    if(!peer) 
    {
        throwIOException(env, peer);
        return; /* not an error, freed previously */ 
    }

    jsize bufsize = (*env)->GetArrayLength(env, data);
    jbyte *buf = (*env)->GetByteArrayElements(env, data, NULL);
    int read = hid_read(peer, buf, bufsize);
    (*env)->ReleaseByteArrayElements(env, data, buf, read==-1?JNI_ABORT:0);
    if(read==-1)
    {
        throwIOException(env, peer);
        return; /* not an error, freed previously */ 
    }
    return read;
}

JNIEXPORT void JNICALL Java_com_codeminders_hidapi_HIDDevice_enableBlocking
  (JNIEnv *env, jobject self)
{
    hid_device *peer = getPeer(env, self);
    if(!peer)
    {
        throwIOException(env, peer);
        return; /* not an error, freed previously */ 
    }
    int res = hid_set_nonblocking(peer,0);
    if(res!=0)
    {
        throwIOException(env, peer);
        return; /* not an error, freed previously */ 
    }
}

JNIEXPORT void JNICALL Java_com_codeminders_hidapi_HIDDevice_disableBlocking
  (JNIEnv *env, jobject self)
{
    hid_device *peer = getPeer(env, self);
    if(!peer)
    {
        throwIOException(env, peer);
        return; /* not an error, freed previously */ 
    }
    int res = hid_set_nonblocking(peer, 1);
    if(res!=0)
    {
        throwIOException(env, peer);
        return; /* not an error, freed previously */ 
    }
}

JNIEXPORT jint JNICALL Java_com_codeminders_hidapi_HIDDevice_sendFeatureReport
  (JNIEnv *env, jobject self, jbyteArray data)
{
    hid_device *peer = getPeer(env, self);
    if(!peer)
    {
        throwIOException(env, peer);
        return; /* not an error, freed previously */ 
    }
    jsize bufsize = (*env)->GetArrayLength(env, data);
    jbyte *buf = (*env)->GetByteArrayElements(env, data, NULL);
    int res = hid_send_feature_report(peer, buf, bufsize);
    if(res!=0)
    {
        throwIOException(env, peer);
        return; /* not an error, freed previously */ 
    }
    
    return res;
}

JNIEXPORT jint JNICALL Java_com_codeminders_hidapi_HIDDevice_getFeatureReport
  (JNIEnv *env, jobject self, jbyteArray data)
{
    hid_device *peer = getPeer(env, self);
    if(!peer)
    {
        throwIOException(env, peer);
        return; /* not an error, freed previously */ 
    }

    jsize bufsize = (*env)->GetArrayLength(env, data);
    jbyte *buf = (*env)->GetByteArrayElements(env, data, NULL);
    int res = hid_read(peer, buf, bufsize);
    (*env)->ReleaseByteArrayElements(env, data, buf, res==-1?JNI_ABORT:0);

    if(res!=0)
    {
        throwIOException(env, peer);
        return; /* not an error, freed previously */ 
    }
    
    return res;
}

JNIEXPORT jstring JNICALL Java_com_codeminders_hidapi_HIDDevice_getManufacturerString
  (JNIEnv *env, jobject self)
{
    hid_device *peer = getPeer(env, self);
    if(!peer)
    {
        throwIOException(env, peer);
        return NULL; /* not an error, freed previously */ 
    }

    wchar_t data[MAX_BUFFER_SIZE];
    int res = hid_get_manufacturer_string(peer, data, MAX_BUFFER_SIZE);
    if(res < 0)
    {
        /* We decided not to treat this as an error, but return an empty string in this case
        throwIOException(env, peer);
        return NULL;
        */
        data[0] = 0;
    }
        
    char *u8 = convertToUTF8(env, data);
    jstring string = (*env)->NewStringUTF(env, u8);
    free(u8);
    
    return string;
}

#include <stdlib.h>

JNIEXPORT jstring JNICALL Java_com_codeminders_hidapi_HIDDevice_getProductString
  (JNIEnv *env, jobject self)
{
    hid_device *peer = getPeer(env, self);
    if(!peer)
    {
        throwIOException(env, peer);
        return NULL; /* not an error, freed previously */ 
    }

    wchar_t data[MAX_BUFFER_SIZE];
    int res = hid_get_product_string(peer, data, MAX_BUFFER_SIZE);
    if(res < 0)
    {
        /* We decided not to treat this as an error, but return an empty string in this case
        throwIOException(env, peer);
        return NULL;
        */
        data[0] = 0;
    }
       
    char *u8 = convertToUTF8(env, data);
    jstring string = (*env)->NewStringUTF(env, u8);
    free(u8);
    
    return string;
}

JNIEXPORT jstring JNICALL Java_com_codeminders_hidapi_HIDDevice_getSerialNumberString
  (JNIEnv *env, jobject self)
{
    hid_device *peer = getPeer(env, self);
    if(!peer)
    {
        throwIOException(env, peer);
        return NULL; /* not an error, freed previously */ 
    }

    wchar_t data[MAX_BUFFER_SIZE];
    int res = hid_get_serial_number_string(peer, data, MAX_BUFFER_SIZE);
    if(res < 0)
    {
        /* We decided not to treat this as an error, but return an empty string in this case
        throwIOException(env, peer);
        return NULL;
        */
        data[0] = 0;
    }
        
    char *u8 = convertToUTF8(env, data);
    jstring string = (*env)->NewStringUTF(env, u8);
    free(u8);
    
    return string;
}

JNIEXPORT jstring JNICALL Java_com_codeminders_hidapi_HIDDevice_getIndexedString
  (JNIEnv *env, jobject self, jint index)
{
    hid_device *peer = getPeer(env, self);
    if(!peer)
    {
        throwIOException(env, peer);
        return NULL; /* not an error, freed previously */ 
    }

    wchar_t data[MAX_BUFFER_SIZE];
    int res = hid_get_indexed_string(peer, index, data, MAX_BUFFER_SIZE);
    if(res < 0)
    {
        /* We decided not to treat this as an error, but return an empty string in this case
        throwIOException(env, peer);
        return NULL;
        */
        data[0] = 0;
    }
        
    char *u8 = convertToUTF8(env, data);
    jstring string = (*env)->NewStringUTF(env, u8);
    free(u8);
    
    return string;
}
