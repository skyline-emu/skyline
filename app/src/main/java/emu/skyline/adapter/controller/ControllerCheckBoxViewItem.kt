/*
 * SPDX-License-Identifier: MPL-2.0
 * Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)
 */

package emu.skyline.adapter.controller

import android.view.ViewGroup
import androidx.core.view.isGone
import emu.skyline.adapter.GenericListItem
import emu.skyline.adapter.GenericViewHolder
import emu.skyline.adapter.ViewBindingFactory
import emu.skyline.adapter.inflater
import emu.skyline.databinding.ControllerCheckboxItemBinding

object ControllerCheckBoxBindingFactory : ViewBindingFactory {
    override fun createBinding(parent : ViewGroup) = ControllerCheckboxItemBinding.inflate(parent.inflater(), parent, false)
}

class ControllerCheckBoxViewItem(var title : String, var summary : String, var checked : Boolean, private val onCheckedChange : (item : ControllerCheckBoxViewItem, position : Int) -> Unit) : GenericListItem<ControllerCheckboxItemBinding>() {
    override fun getViewBindingFactory() = ControllerCheckBoxBindingFactory

    override fun bind(holder : GenericViewHolder<ControllerCheckboxItemBinding>, position : Int) {
        holder.binding.textTitle.isGone = title.isEmpty()
        holder.binding.textTitle.text = title
        holder.binding.textSubtitle.isGone = summary.isEmpty()
        holder.binding.textSubtitle.text = summary
        holder.binding.checkbox.isChecked = checked
        holder.itemView.setOnClickListener {
            checked = !checked
            onCheckedChange.invoke(this, position)
        }
    }

    override fun areItemsTheSame(other : GenericListItem<ControllerCheckboxItemBinding>) = other is ControllerCheckBoxViewItem

    override fun areContentsTheSame(other : GenericListItem<ControllerCheckboxItemBinding>) = other is ControllerCheckBoxViewItem && title == other.title && summary == other.summary && checked == other.checked
}
