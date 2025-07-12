# NymphRPC Binary Protocol

This document defines the NymphRPC protocol. It's a binary protocol for remote procedure calls.

Details:
- Little Endian format: all binary messages are in LE byte format.
- All fields are 8 to 64 bytes long, in unsigned integer format.

---

## Protocol 

Each binary message starts with the same header, followed by a message-specific payload.

**Header**

<pre>
uint32		Signature: DRGN (0x4452474e)
uint32		Total message bytes following this field.
uint8		Protocol version (0x00).
uint32		Method ID: identifier of the remote function.
uint32		Flags (see _Flags_ section).
uint64		Message ID. Simple incrementing global counter.
</pre>


**Flags**

<pre>
0x01	Reply message.
0x02	Exception message.
0x04	Callback message.
</pre>


**Regular message**

<pre>
&lt;header&gt;
&lt;..&gt;	Serialised values.
uint8		Message end. None type (0x01). See 'Types' section.
</pre>


**Reply message**

<pre>
&lt;header&gt;
uint64		ReplyTo ID: message ID that this is in response to.
&lt;..&gt;		Serialised reply.
uint8		Message end. None type (0x01). See 'Types' section.
</pre>


**Exception message**

<pre>
&lt;header&gt;
uint64		ReplyTo ID: message ID that this is in response to.
uint32		Exception ID.
uint8		Message end. None type (0x01). See 'Types' section.
</pre>


**Callback message**

<pre>
&lt;header&gt;
uint8		String typecode. (0x10)
uint8-64	String length.
uint64		ReplyTo ID: message ID that this is in response to.
&lt;..&gt;		Callback name (String).
&lt;..&gt;		Serialised values.
uint8		Message end. None typecode (0x01). See 'Types' section.
</pre>

----

## Types

Types in NymphRPC are divided into internal and external types. The external types are the types used by an application, while the internal ones are used by NymphRPC itself.

**Internal typecodes**

- Unsigned integers are defined as <i>Uint*</i>.
- Signed integers are defined as <i>Sint*</i>.
- Float is 32-bit floating point.
- Double is 64-bit floating point.
- See the <i>Complex types</i> section for details on specific types.

<pre>
Null			0x00
None			0x01
Boolean false	0x02
Boolean true	0x03
Uint8			0x04
Sint8			0x05
Uint16			0x06
Sint16			0x07
Uint32			0x08
Sint32			0x09
Uint64			0x0a
Sint64			0x0b
Float			0x0c
Double			0x0d
Array			0x0e
Empty string	0x0f
String			0x10
Struct			0x11
Void			0x12
</pre>


**External typecodes**

<pre>

</pre>

----

## Complex types